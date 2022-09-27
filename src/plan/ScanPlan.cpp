#include "plan/ScanPlan.h"
#include "base/lang/ASTNode.h"
#include "base/lang/QueryStmt.h"
#include "base/system/Observer.h"
#include "gqlite.h"
#include "StorageEngine.h"
#include <filesystem>
#include "json.hpp"
#include "Type/Datetime.h"

GScanPlan::GScanPlan(std::map<std::string, GVirtualNetwork*>& networks, GStorageEngine* store, GQueryStmt* stmt)
:GPlan(networks, store)
, _queryType(QueryType::SimpleScan)
, _state(ScanState::Stop)
{
  auto* ptr = stmt->graph();
  if (ptr) {
    _graph = GetString(ptr);
  }
  else {
    auto jsn = store->getSchema();
    _graph = jsn[SCHEMA_GRAPH_NAME] ;
  }

  auto* query = stmt->query();
  parseGroup(query);
  parseConditions(stmt->where());
}

GScanPlan::GScanPlan(std::map<std::string, GVirtualNetwork*>& networks, GStorageEngine* store, GASTNode* condition, const std::string& group)
  :GPlan(networks, store)
  , _queryType(QueryType::SimpleScan)
{
  auto jsn = store->getSchema();
  _graph = jsn[SCHEMA_GRAPH_NAME];

  _queries.push_back(group);
  parseConditions(condition);
}

GScanPlan::~GScanPlan()
{
  for (IObserver* observer: _observers)
  {
    delete observer;
  }
  _observers.clear();
  for (EntityNode* node : _pattern._nodes) {
    delete node;
  }
  _pattern._nodes.clear();
}

void GScanPlan::addObserver(IObserver* observer)
{
  _observers.push_back(observer);
}

int GScanPlan::prepare()
{
  if (_graph.empty()) return ECode_Graph_Not_Exist;
  if (!std::filesystem::exists(_graph)) {
    return ECode_Graph_Not_Exist;
  }
  return ECode_Success;
}

int GScanPlan::execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>& processor) {
  _interrupt.store(false);
  start();
#ifdef GQLITE_MULTI_THREAD
  _worker = std::thread(&GScanPlan::scan, this);
#else
  scan(processor);
#endif
  return ECode_Success;
}

int GScanPlan::interrupt()
{
  _interrupt.store(true);
  _worker.join();
  return ECode_Success;
}

void GScanPlan::start()
{
  _state = ScanState::Scanning;
  _scanRecord._itr = _queries.end();
}

void GScanPlan::pause()
{
  _state = ScanState::Pause;
}

void GScanPlan::goon()
{
  _state = ScanState::Scanning;
}

void GScanPlan::stop()
{
  _state = ScanState::Stop;
}

int GScanPlan::scan(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>& cb)
{
  if (_queries.size() == 0) return ECode_Success;
  std::vector<std::string>::iterator itr = _queries.begin();
  GStorageEngine::cursor cursor = _store->getCursor(*itr);
  auto data = cursor.to_first(false);
  if (_scanRecord._itr != _queries.end()) {
    itr = _scanRecord._itr;
    cursor = std::move(_scanRecord._cursor);
  }
  while (itr != _queries.end())
  {
    std::string group = *itr;
    KeyType type = _store->getKeyType(group);
    while (data)
    {
      std::string str((char*)data.value.byte_ptr(), data.value.size());
      //printf("scan: %s\n", str.c_str());
      switch (_queryType)
      {
      case GScanPlan::QueryType::SimpleScan:
      {
        gkey_t vKey = getKey(type, data.key);
        nlohmann::json jsn = nlohmann::json::parse(str);
        std::vector<std::string>::iterator aitr;
        if (_pattern._nodes.size()) {
          aitr = _pattern._nodes[0]->_attrs.begin();
        }
        bool result = true;
        auto& andPreds = _pattern._node_predicates[(long)LogicalPredicate::And];
        for (auto predItr = andPreds.begin(), end = andPreds.end(); predItr != end; ++predItr) {
          result &= (*predItr).visit(
            [&vKey](std::function<bool(const gkey_t&)> op) {
              return op(vKey);
            },
            [&jsn, &aitr, this](std::function<bool(const attribute_t&)> op) {
              auto value = jsn[*aitr];
              bool ret = predict(op, value);
              ++aitr;
              return ret;
            });
          if (!result) break;
        }
        for (auto& pred: _pattern._node_predicates[(long)LogicalPredicate::Or])
        {
        }
        if (result) {
          std::string k((char*)data.key.byte_ptr(), data.key.size());
          for (IObserver* observer: _observers) {
            observer->update(type, k, jsn);
          }
          beautify(jsn);
          if (cb) cb(type, k, jsn.dump());
        }
      }
        break;
      case GScanPlan::QueryType::NNSearch:
        break;
      case GScanPlan::QueryType::Match:
        break;
      case GScanPlan::QueryType::Inference:
        break;
      default:
        break;
      }
      data = cursor.to_next(false);
      if (pauseExit(cursor, itr) || stopExit())
        return ECode_Success;
    }
    ++itr;
    if (itr == _queries.end()) break;
    cursor = _store->getCursor(*itr);
    data = cursor.to_first(false);
  }
  return ECode_Success;
}

void GScanPlan::parseGroup(GASTNode* query)
{
  if (query->_nodetype == NodeType::ArrayExpression) {
    GArrayExpression* arr = reinterpret_cast<GArrayExpression*>(query->_value);
    for (auto itr = arr->begin(), end = arr->end(); itr != end; ++itr) {
      switch ((*itr)->_nodetype)
      {
      case NodeType::MemberExpression:
        break;
      case NodeType::Literal:
        _queries.emplace_back(GetString(*itr));
        break;
      default:
        break;
      }
    }
  }
  else if (query->_nodetype == NodeType::Literal) {
    _queries.emplace_back(GetString(query));
  }
}

void GScanPlan::parseConditions(GASTNode* conditions)
{
  if (conditions == nullptr) return;
  EntityNode* node = new EntityNode;
  PatternVisitor visitor(_pattern, node);
  std::list<NodeType> lNodes;
  accept(conditions, visitor, lNodes);
  _pattern._nodes.push_back(node);
  if (_pattern._edges.size() != 0) _queryType = QueryType::Match;
}

bool GScanPlan::pauseExit(GStorageEngine::cursor& cursor, std::vector<std::string>::iterator itr)
{
  if (_state == ScanState::Pause) {
    _scanRecord._itr = itr;
    _scanRecord._cursor = std::move(cursor);
    return true;
  }
  return false;
}

bool GScanPlan::stopExit()
{
  if (_state == ScanState::Stop) {
    return true;
  }
  return false;
}

gkey_t GScanPlan::getKey(KeyType type, mdbx::slice& slice)
{
  gkey_t vKey;
  if (type == KeyType::Integer) {
    auto v = *(uint64_t*)slice.byte_ptr();
    vKey = (uint64_t)v;
  }
  else {
    std::string key((char*)slice.byte_ptr(), slice.size());
    vKey = key;
  }
  return vKey;
}

bool GScanPlan::predict(const std::function<bool(const attribute_t&)>& op, const nlohmann::json& attr) const
{
  bool ret = false;
  switch ((nlohmann::json::value_t)attr) {
  case nlohmann::json::value_t::number_float:
  case nlohmann::json::value_t::number_integer:
    ret = op((double)attr);
    break;
  case nlohmann::json::value_t::string:
    ret = op((std::string)attr);
    break;
  case nlohmann::json::value_t::object:
    if (attr.count(OBJECT_TYPE_NAME)) {
      switch (AttributeKind(attr[OBJECT_TYPE_NAME])) {
      case AttributeKind::Datetime:
      {
        ret = op((double)attr["value"]);
      }
      break;
      default:
        break;
      }
    }
    else {}
    break;
  default:
    break;
  }
  return ret;
}

void GScanPlan::beautify(nlohmann::json& input)
{
  if (input.empty() || input.is_null()) return;
  if (input.is_object()) {
    if (input.count(OBJECT_TYPE_NAME)) {
      switch (AttributeKind(input[OBJECT_TYPE_NAME])) {
      case AttributeKind::Datetime:
        input = std::string("0d") + std::to_string((time_t)input["value"]);
        return;
      case AttributeKind::Vector:
        input = input["value"];
        return;
      default:
        break;
      }
    }
    for (auto itr = input.begin(); itr != input.end(); ++itr)
    {
      beautify(itr.value());
    }
  }
  else if (input.is_array()) {
    for (auto itr = input.begin(); itr != input.end(); ++itr)
    {
      beautify(itr.value());
    }
  }
}

VisitFlow GScanPlan::PatternVisitor::apply(GVertexDeclaration* stmt, std::list<NodeType>& path)
{
  if (!stmt->vertex()) {
    // equal id
    std::string key = GetString(stmt->key());
    predicate_t pred = static_cast<std::function<bool(const gkey_t&)>>([key](const Variant<std::string, uint64_t>& input)->bool {
      bool ret = input.visit([key](uint64_t i) -> bool {
        return key == std::to_string(i);
        },
        [key](std::string s) -> bool {
          return key == s;
        });
      return ret;
    });
    _pattern._node_predicates[(long)LogicalPredicate::And].push_back(pred);
  }
  return VisitFlow::Children;
}

VisitFlow GScanPlan::PatternVisitor::apply(GLiteral* stmt, std::list<NodeType>& path)
{
  return VisitFlow::Children;
}

VisitFlow GScanPlan::PatternVisitor::apply(GArrayExpression* stmt, std::list<NodeType>& path)
{
  // vertex condition or others
  for (auto itr = stmt->begin(), end = stmt->end(); itr != end; ++itr) {
    GASTNode* ptr = (*itr)->_children;
    for (size_t indx = 0; indx < (*itr)->_size; ++indx) {
      GASTNode* children = ptr + indx;
      accept(children, *this, path);
    }
  }
  return VisitFlow::Children;
}

VisitFlow GScanPlan::PatternVisitor::apply(GASTNode* stmt, std::list<NodeType>& path)
{
  // object of vertex condition
  return VisitFlow::Children;
}

VisitFlow GScanPlan::PatternVisitor::apply(GProperty* stmt, std::list<NodeType>& path)
{
  std::string key = stmt->key();
  static std::string last_key;
  auto getLiteral = [](GASTNode* node) {
    attribute_t v;
    if (node->_nodetype != NodeType::Literal) return v;
    GLiteral* literal = (GLiteral*)node->_value;
    switch (literal->kind()) {
    case AttributeKind::String:
      v = literal->raw();
      break;
    case AttributeKind::Datetime:
      v = (double)atoll(literal->raw().c_str());
      break;
    case AttributeKind::Integer:
    case AttributeKind::Number:
      v = (double)atof(literal->raw().c_str());
      break;
    default:
      break;
    }
    return v;
  };
  if (key == "lt") {
    _node->_attrs.push_back(last_key);
    attribute_t attr = getLiteral(stmt->value());
    predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([attr](const attribute_t& input)->bool {
      return input < attr;
      });
    _pattern._node_predicates[(long)LogicalPredicate::And].push_back(pred);
  }
  else if (key == "gt") {
    _node->_attrs.push_back(last_key);
    attribute_t attr = getLiteral(stmt->value());
    predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([attr](const attribute_t& input)->bool {
      return input > attr;
      });
    _pattern._node_predicates[(long)LogicalPredicate::And].push_back(pred);
  }
  else if (key == "lte") {
    _node->_attrs.push_back(last_key);
    attribute_t attr = getLiteral(stmt->value());
    predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([attr](const attribute_t& input)->bool {
      return input <= attr;
      });
    _pattern._node_predicates[(long)LogicalPredicate::And].push_back(pred);
  }
  else if (key == "gte") {
    _node->_attrs.push_back(last_key);
    attribute_t attr = getLiteral(stmt->value());
    predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([attr](const attribute_t& input)->bool {
      return input >= attr;
      });
    _pattern._node_predicates[(long)LogicalPredicate::And].push_back(pred);
  }
  else if (key == "id") {
    std::string key = GetString(stmt->value());
    predicate_t pred = static_cast<std::function<bool(const gkey_t&)>>([key](const Variant<std::string, uint64_t>& input)->bool {
      bool ret = input.visit([key](uint64_t i) -> bool {
        return key == std::to_string(i);
        },
        [key](std::string s) -> bool {
          return key == s;
        });
      return ret;
      });
    _pattern._node_predicates[(long)LogicalPredicate::And].push_back(pred);
  }
  else {
    last_key = key;
    attribute_t attr = getLiteral(stmt->value());
    if (!attr.empty()) {
      _node->_attrs.push_back(last_key);
      predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([attr](const attribute_t& input)->bool {
        return input == attr;
        });
      _pattern._node_predicates[(long)LogicalPredicate::And].push_back(pred);
    }
  }
  return VisitFlow::Children;
}

VisitFlow GScanPlan::VertexJsonVisitor::apply(GProperty* stmt, std::list<NodeType>& path)
{
  return VisitFlow::Children;
}

VisitFlow GScanPlan::VertexJsonVisitor::apply(GVertexDeclaration* stmt, std::list<NodeType>& path)
{
  return VisitFlow::Children;
}

VisitFlow GScanPlan::VertexJsonVisitor::apply(GLiteral* stmt, std::list<NodeType>& path)
{
  return VisitFlow::Children;
}

VisitFlow GScanPlan::VertexJsonVisitor::apply(GArrayExpression* stmt, std::list<NodeType>& path)
{
  return VisitFlow::Children;
}
