#include "plan/query/ScanPlan.h"
#include "Context.h"
#include "base/lang/AST.h"
#include "base/lang/ASTNode.h"
#include "base/lang/QueryStmt.h"
#include "base/lang/ObjectFunction.h"
#include "base/lang/visitor/WalkVisitor.h"
#include "base/system/Coroutine.h"
#include "base/system/Observer.h"
#include "gqlite.h"
#include "StorageEngine.h"
#include <float.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include <set>
#include "json.hpp"
#include "gutil.h"
#include "Type/Datetime.h"
#include "base/math/Distance.h"
#include "base/gvm/GVM.h"
#include "base/gvm/Compiler.h"

#if __cplusplus > 201700
#include <filesystem>
using namespace std;
#elif __cplusplus > 201300
#include <experimental/filesystem>
using namespace std::experimental;
#elif __cplusplus > 201103 
#endif

GScanPlan::GScanPlan(GContext* context, GQueryStmt* stmt)
:GPlan(context->_graph, context->_storage, context->_schedule)
, _queryType(QueryType::SimpleScan)
, _state(ScanState::Stop)
{
  auto* ptr = stmt->graph();
  if (ptr) {
    _graph = GetString(ptr);
  }
  else {
    auto jsn = _store->getSchema();
    _graph = jsn[SCHEMA_GRAPH_NAME];
  }

  auto* query = stmt->query();
  parseGroup(query);
  parseConditions(stmt->where());
}

GScanPlan::GScanPlan(GContext* context, GListNode* condition, const std::string& group)
:GPlan(context->_graph, context->_storage, context->_schedule)
  , _queryType(QueryType::SimpleScan)
  ,_group(group)
{
  auto jsn = _store->getSchema();
  _graph = jsn[SCHEMA_GRAPH_NAME];

  _queries[0].push_back({ FLT_MAX, group });
  parseConditions(condition);
}

GScanPlan::~GScanPlan()
{
  for (IObserver* observer: _observers)
  {
    delete observer;
  }
  _observers.clear();
  for (int index = 0; index < (long)LogicalPredicate::Max; ++index) {
    auto& graphPattern = _where._patterns[index];
    for (EntityNode* node : graphPattern._nodes) {
      delete node;
    }
    graphPattern._nodes.clear();

    std::set< EntityNode*> restNodes;
    for (EntityEdge* edge : graphPattern._edges) {
      if (edge->_start) restNodes.insert(edge->_start);
      if (edge->_end) restNodes.insert(edge->_end);
      delete edge;
    }
    graphPattern._edges.clear();

    for (auto ptr : restNodes) {
      delete ptr;
    }
  }
}

void GScanPlan::addObserver(IObserver* observer)
{
  _observers.push_back(observer);
}

int GScanPlan::prepare()
{
  if (_graph.empty()) return ECode_Graph_Not_Exist;
  std::string curDB = _store->getPath();
  if (!filesystem::exists(curDB) || _graph != _store->getSchema()[SCHEMA_GRAPH_NAME]) {
    return ECode_Graph_Not_Exist;
  }
  if (!_store->isMapExist(_group)) return ECode_Group_Not_Exist;
  switch (_queryType)
  {
  case QueryType::SimpleScan:
  {
    // check if index exist
    for (int index = 0; index < (long)LogicalPredicate::Max; ++index) {
      ScanPlans plans;
      auto& pattern = _where._patterns[index];
      if (pattern._nodes.size()) {
        auto begin = pattern._nodes[0]->_attrs.begin();
        auto end = pattern._nodes[0]->_attrs.end();
        for (auto ptr = begin; ptr != end; ++ptr) {
          plans.push_back({ 0, _group + ":" + *ptr});
        }
      }
      plans.insert(plans.end(), _queries[index].begin(), _queries[index].end());
      _queries[index] = evaluate(plans);
      if (_queries[index].size()) {
        std::sort(_queries[index].begin(), _queries[index].end(), [](const PlanInfo& left, const PlanInfo& right) {
          return left.cost < right.cost;
          });
      }
    }
  }
  break;
  default:
    break;
  }
  return ECode_Success;
}

int GScanPlan::execute(GVM* gvm, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>& processor) {
  _interrupt.store(false);
  start();
#ifdef GQLITE_MULTI_THREAD
  _worker = std::thread(&GScanPlan::scan, this);
#else
  _gvm = gvm;
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
  _scanRecord._op = LogicalPredicate::And;
  _scanRecord._itr = _queries[0].end();
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

int GScanPlan::scan(const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>& cb)
{
  if (_queries[0].size() == 0 && _queries[1].size() == 0) return ECode_Success;
  int ret = scan();
  if (ret == ECode_Query_Pause || ret == ECode_Query_Stop) return ECode_Success;
  if (ret != ECode_Success) return ECode_Fail;
  for (int index = 0; index < (long)LogicalPredicate::Max; ++index) {
    if (_scanRecord._op != (LogicalPredicate)index) continue;
    ScanPlans::iterator itr = _queries[index].begin();
    GStorageEngine::cursor cursor = _store->getMapCursor(itr->_group);
    auto data = cursor.to_first(false);
    if (_scanRecord._itr != _queries[index].end()) {
      itr = _scanRecord._itr;
      cursor = std::move(_scanRecord._cursor);
    }
    while (itr != _queries[index].end())
    {
      std::string group = itr->_group;
      KeyType type = _store->getKeyType(group);
      while (data)
      {
        std::string str((char*)data.value.byte_ptr(), data.value.size());
        //printf("scan: %s\n", str.c_str());
        switch (_queryType)
        {
        case QueryType::SimpleScan:
        {
          gkey_t vKey = getKey(type, data.key);
          nlohmann::json jsn = nlohmann::json::parse(str);
          try {
            if (_scanAll || (!_scanAll && predict(type, vKey, jsn))) {
              std::string k((char*)data.key.byte_ptr(), data.key.size());
              for (IObserver* observer : _observers) {
                observer->update(type, k, jsn);
              }
              if (cb) cb(type, k, jsn, ECode_Success);
            }
          }
          catch (gql::variant_bad_cast& e) {
            if (cb) cb(type, e.what(), jsn, ECode_GQL_Type_Not_Match);
            else {
              fmt::print(fmt::fg(fmt::color::yellow), "{}", e.what());
            }
          }
          
        }
        break;
        case QueryType::NNSearch:
          break;
        case QueryType::Match:
          break;
        case QueryType::Inference:
          break;
        default:
          break;
        }
        data = cursor.to_next(false);
        if (pauseExit(cursor, itr) || stopExit())
          return ECode_Success;
      }
      ++itr;
      if (itr == _queries[index].end()) break;
      cursor = _store->getMapCursor(itr->_group);
      data = cursor.to_first(false);
    }
  }
  
  return ECode_Success;
}

int GScanPlan::scan()
{
  if (_queries[0].size() == 0 && _queries[1].size() == 0) return ECode_Success;
  for (int index = 0; index < (long)LogicalPredicate::Max; ++index) {
    if (_scanRecord._op != (LogicalPredicate)index || _queries[index].size() == 1) {
      continue;
    }
    auto itr = _queries[index].begin();

    std::string groupIndex = itr->_group;
    GStorageEngine::cursor cursor = _store->getIndexCursor(groupIndex);
    if (_scanRecord._itr != _queries[index].end()) {
      itr = _scanRecord._itr;
      cursor = std::move(_scanRecord._cursor);
    }
    for (; itr != _queries[index].end(); ) {
      if (itr->cost != 0) {
        // get predictor
        int idx = 0;
        auto& andPattern = _where._patterns[index];
        if (andPattern._node_predicates.size()) {
          auto& andAttr = andPattern._nodes[0]->_attrs;
          for (auto ptr = andAttr.begin(), end = andAttr.end(); ptr != end; ++ptr, ++idx) {
            if (*ptr == groupIndex) break;
          }
          auto& predictsAnd = andPattern._node_predicates;
          for (auto itr = predictsAnd.begin(), end = predictsAnd.end(); itr != end; ++itr) {

          }
          std::list<std::string> keys;
          switch (_store->getIndexType(groupIndex)) {
          case IndexType::Number:
          {
            double from = 0;
            double to = 100;
            std::string sf((char*)&from, sizeof(double));
            std::string st((char*)&to, sizeof(double));
            auto data = cursor.to_first(false);
            while (data) {
              double value = *(double*)data.key.byte_ptr();
              if (value >= from && value < to) {

              }
              data = cursor.to_next(false);
            }
          }
          break;
          case IndexType::Word:
          {

          }
          break;
          case IndexType::Vector:
            break;
          default:
            break;
          }
        }
        if (pauseExit(cursor, itr)) return ECode_Query_Pause;
        if (stopExit()) return ECode_Query_Stop;
      }
      itr = _queries[index].erase(itr);
      _scanRecord._itr = _queries[index].end();
      // last query is group
      if (_queries[index].size() == 1) break;
    }
  }
  return ECode_Success;
}

void GScanPlan::parseGroup(GListNode* query)
{
  if (query->_nodetype == NodeType::ArrayExpression) {
    GArrayExpression* arr = reinterpret_cast<GArrayExpression*>(query->_value);
    for (auto itr = arr->begin(), end = arr->end(); itr != end; ++itr) {
      switch ((*itr)->_nodetype)
      {
      case NodeType::MemberExpression:
      {
        GMemberExpression* expr = (GMemberExpression*)(*itr)->_value;
        _group = expr->GetObjectName();
        initQueryGroups(_group);
      }
        break;
      case NodeType::Literal:
        initQueryGroups(GetString(*itr));
        break;
      default:
        break;
      }
    }
  }
  else if (query->_nodetype == NodeType::Literal) {
    _group = GetString(query);
    initQueryGroups(_group);
  }
}

void GScanPlan::parseConditions(GListNode* conditions)
{
  if (conditions == nullptr) {
    _scanAll = true;
    return;
  }
  _scanAll = false;

  // GWhereVisitor visitor(_where);
  PatternVisitor visitor(_where);
  std::list<NodeType> lNodes;
  accept(conditions, &visitor, lNodes);
  for (int i = 0; i < (int)LogicalPredicate::Max; ++i) {
    if (_where._patterns[visitor._index[i]]._edges.size() > 1) {
      _queryType = QueryType::Match;
    }
    else if (_where._patterns[visitor._index[i]]._edges.size() == 1) {
      // query one kind of edge
    }
    else { // query type is simple scan
      if (visitor._attrs[i].size()) {
        EntityNode* node = new EntityNode;
        node->_attrs = visitor._attrs[i];
        _where._patterns[visitor._index[i]]._nodes.push_back(node);
      }
    }
  }
  
}

bool GScanPlan::pauseExit(GStorageEngine::cursor& cursor, ScanPlans::iterator itr)
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
  switch (type) {
  case KeyType::Byte:
  {
    std::string key((char*)slice.byte_ptr(), slice.size());
    vKey = key;
  }
    break;
  case KeyType::Integer:
  {
    auto v = *(uint64_t*)slice.byte_ptr();
    vKey = (uint64_t)v;
  }
    break;
  case KeyType::Edge:
  {
    std::string key((char*)slice.byte_ptr(), slice.size());
    //auto edge = gql::to_edge_id(key);
    vKey = key;
  }
    break;
  default:
    break;
  }
  return vKey;
}

bool GScanPlan::predictVertex(gkey_t key, nlohmann::json& row)
{
  bool result = true;
  if (_compiler) {
    if (_gvm->interpret(_compiler->_func) == ECode_Success) {
      Value res = _gvm->result();
    }
  }
  std::vector<std::string>::iterator aitr;
  for (int index = 0; index < (long)LogicalPredicate::Max; ++index) {
    if (_where._patterns[index]._nodes.size()) {
      aitr = _where._patterns[index]._nodes[0]->_attrs.begin();
    }
    auto& opPreds = _where._patterns[index]._node_predicates;
    for (auto predItr = opPreds.begin(), end = opPreds.end(); predItr != end; ++predItr) {
      result &= (*predItr).visit(
        [&key](std::function<bool(const gkey_t&)> op) {
          return op(key);
        },
        [&row, &aitr, this](std::function<bool(const attribute_t&)> op) {
          auto value = row[*aitr];
          bool ret = predict(op, value);
          ++aitr;
          return ret;
        });
      if (index == 0 && !result) break;
      if (index == 1 && result) break;
    }
  }
  return result;
}

bool GScanPlan::predict(const std::function<bool(const attribute_t&)>& op, const nlohmann::json& attr) const
{
  bool ret = false;
  switch ((nlohmann::json::value_t)attr) {
  case nlohmann::json::value_t::number_float:
  case nlohmann::json::value_t::number_integer:
  case nlohmann::json::value_t::number_unsigned:
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
      case AttributeKind::Vector:
      {
        std::vector<double> dv = attr["value"];
        attribute_t a = dv;
        ret = op(a);
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

bool GScanPlan::predict(KeyType type, gkey_t key, nlohmann::json& row)
{
  switch (type)
  {
  case KeyType::Edge: {
    return predictEdge(key, row);
  }
  default:
    return predictVertex(key, row);
  }
  
}

void GScanPlan::initQueryGroups(const std::string& group) {
  if (group == "*") {
    auto& allGroups = _store->getSchema()[SCHEMA_CLASS];
    std::string all = allGroups.dump();
    for (auto& group: allGroups.items()) {
      std::string name = group.key();
      if (name == MAP_BASIC) continue;
      _queries[0].push_back({ 0, name });
    }
  }
  else {
    _queries[0].push_back({ 0, _group });
  }
}

bool GScanPlan::predictEdge(gkey_t key, nlohmann::json& row)
{
  auto match_node = [](const std::string node1, const std::string& node2) -> bool {
    // input is `*`, all nodes are correct
    if (node1.empty() || node2.empty()) return true;

    return node1 == node2;
  };
  auto match_node_int = [](const std::string node1, uint64_t node2) {
    if (node1.empty()) return true;

    uint64_t value = strtoull(node1.c_str(), nullptr, 10);
    return value == node2;
  };
  gql::edge_id eid = gql::to_edge_id(key.Get<std::string>());
  gkey_t from, to;
  get_from_to(eid, from, to);
  std::vector<std::string>::iterator aitr;
  for (int index = 0; index < (long)LogicalPredicate::Max; ++index) {
    auto& edges = _where._patterns[index]._edges;
    for (auto itr = edges.begin(); itr != edges.end(); ++itr) {
      auto& edge = *itr;
      if (eid._direction == edge->_direction) {
        auto& start = edge->_start->_label;
        auto& end = edge->_end->_label;
        bool from_result = eid._from_type ? match_node(start, from.Get<std::string>()) : match_node_int(start, from.Get<uint64_t>());
        bool to_result = eid._to_type ? match_node(end, to.Get<std::string>()) : match_node_int(end, to.Get<uint64_t>());

        if (eid._direction == false) {
          // swap start and end
          if (!from_result || !to_result) {
            from_result = eid._from_type ? match_node(end, from.Get<std::string>()) : match_node_int(end, from.Get<uint64_t>());
            to_result = eid._to_type ? match_node(start, to.Get<std::string>()) : match_node_int(start, to.Get<uint64_t>());
          }
        }
        release_edge_id(eid);
        return from_result && to_result;
      }
    }
  }
  release_edge_id(eid);
  return false;
}

GScanPlan::ScanPlans GScanPlan::evaluate(const ScanPlans& props)
{
  ScanPlans indexes;
  for (const PlanInfo& item : props) {
    size_t diff = _store->estimate(item._group);
    if (diff == std::numeric_limits<size_t>::max()) continue;
    indexes.push_back({ 1.0f * diff, item._group });
  }
  return indexes;
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
    _where._patterns[_isAnd ? 0 : 1]._node_predicates.push_back(pred);
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
    accept(*itr, this, path);
    //GListNode* ptr = (*itr)->_children;
    //for (size_t indx = 0; indx < (*itr)->_size; ++indx) {
    //  GListNode* children = ptr + indx;
    //  accept(children, *this, path);
    //}
  }
  return VisitFlow::SkipCurrent;
}

VisitFlow GScanPlan::PatternVisitor::apply(GProperty* stmt, std::list<NodeType>& path)
{
  std::string key = stmt->key();
  static std::string last_key;
  
  int index = _isAnd ? 0 : 1;
  if (key == "lt") {
    _attrs[index].push_back(last_key);
    attribute_t attr;
    if (!GetLiteral(stmt->value(), attr)) {
      //return accept(stmt->value(), this, path);
      return VisitFlow::SkipCurrent;
    }

    predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([attr](const attribute_t& input)->bool {
      return input < attr;
      });
    _where._patterns[index]._node_predicates.push_back(pred);
  }
  else if (key == "gt") {
    _attrs[index].push_back(last_key);
    attribute_t attr;
    if (!GetLiteral(stmt->value(), attr))
      return VisitFlow::SkipCurrent;
    predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([attr](const attribute_t& input)->bool {
      return input > attr;
      });
    _where._patterns[index]._node_predicates.push_back(pred);
  }
  else if (key == "lte") {
    _attrs[index].push_back(last_key);
    attribute_t attr;
    if (!GetLiteral(stmt->value(), attr)) {
      return VisitFlow::SkipCurrent;
      //return accept(stmt->value(), this, path);
    }

    predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([attr](const attribute_t& input)->bool {
      return input <= attr;
      });
    _where._patterns[index]._node_predicates.push_back(pred);
  }
  else if (key == "gte") {
    _attrs[index].push_back(last_key);
    attribute_t attr;
    if (!GetLiteral(stmt->value(), attr))
      return VisitFlow::SkipCurrent;

    predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([attr](const attribute_t& input)->bool {
      return input >= attr;
      });
    _where._patterns[index]._node_predicates.push_back(pred);
  }
  else if (key == "id") {
    std::string value = GetString(stmt->value());
    predicate_t pred = static_cast<std::function<bool(const gkey_t&)>>([value](const Variant<std::string, uint64_t>& input)->bool {
      bool ret = input.visit([value](uint64_t i) -> bool {
        return value == std::to_string(i);
        },
        [value](std::string s) -> bool {
          return value == s;
        });
      return ret;
      });
    _where._patterns[index]._node_predicates.push_back(pred);
  }
  else if (key == "and") {
    _isAnd = true;
  }
  else if (key == "or") {
    _isAnd = false;
  }
  else if (key == "near") {
    auto ptr = stmt->value();
    GObjectFunction* obj = (GObjectFunction*)ptr->_value;
    std::vector<double> vec = GetVector((*obj)[0]);

    GListNode* comp = (*obj)[1];
    GProperty* prop = (GProperty*)comp->_value;
    std::string comparable = prop->key();
    attribute_t attr;
    if (!GetLiteral(prop->value(), attr))
      return VisitFlow::SkipCurrent;

    double right = attr.Get<double>();

    if (comparable == "lt") {
      predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([right, vec](const attribute_t& input)->bool {
        gql::vector_double left = input.Get<gql::vector_double>();
        return gql::distance2(vec, left) < right;
      });
      _where._patterns[index]._node_predicates.push_back(pred);
    }
    else if (comparable == "lte") {
      predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([right, vec](const attribute_t& input)->bool {
        gql::vector_double left = input.Get<gql::vector_double>();
        return gql::distance2(vec, left) <= right;
        });
      _where._patterns[index]._node_predicates.push_back(pred);
    }
    else if (comparable == "gt") {
      predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([right, vec](const attribute_t& input)->bool {
        gql::vector_double left = input.Get<gql::vector_double>();
        return gql::distance2(vec, left) > right;
        });
      _where._patterns[index]._node_predicates.push_back(pred);
    }
    else if (comparable == "gte") {
      predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([right, vec](const attribute_t& input)->bool {
        gql::vector_double left = input.Get<gql::vector_double>();
        return gql::distance2(vec, left) >= right;
        });
      _where._patterns[index]._node_predicates.push_back(pred);
    }
    else {
      fmt::print(fmt::fg(fmt::color::yellow), "not support operator {}\n", comparable);
      predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([right, vec](const attribute_t& input)->bool {
        return false;
        });
      _where._patterns[index]._node_predicates.push_back(pred);
    }
    
    _attrs[index].push_back(last_key);
  }
  else { // group's name
    last_key = key;
    GListNode* value = stmt->value();

    auto addEqualLiteral = [this](GListNode* value, int index) {
      if (value->_nodetype == NodeType::Literal) {
        std::string cond = GetString(value);
        if (cond == "*") {
          _attrs[index].push_back(last_key);
          predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([](const attribute_t& input)->bool {
            return true;
          });
          _where._patterns[index]._node_predicates.push_back(pred);
          return;
        }
      }
      attribute_t attr;
      if (!GetLiteral(value, attr))
        return;
      if (!attr.empty()) {
        _attrs[index].push_back(last_key);
        predicate_t pred = static_cast<std::function<bool(const attribute_t&)>>([attr](const attribute_t& input)->bool {
          return input == attr;
          });
        _where._patterns[index]._node_predicates.push_back(pred);
      }
    };
    if (value->_nodetype == NodeType::Literal) {
      addEqualLiteral(value, index);
    }
    else if (value->_nodetype == NodeType::ArrayExpression) {
      GArrayExpression* arr = (GArrayExpression*)(value->_value);
      for (auto itr = arr->begin(), end = arr->end(); itr != end; ++itr) {
        addEqualLiteral(*itr, (int)LogicalPredicate::Or);
      }
    }
    else {
      return accept(value, this, path);
    }
  }
  return VisitFlow::Children;
}

VisitFlow GScanPlan::PatternVisitor::apply(GWalkDeclaration* stmt, std::list<NodeType>& path)
{
  int index = _isAnd ? 0 : 1;
  auto& grad = _where._patterns[index];
  GWalkVisitor visitor(&grad);
  VisitFlow vf = visitor.apply(stmt,path);
  // if (visitor.walkType() == GraphMatch) _qt = QueryType::Match;
  return vf;
}

VisitFlow GScanPlan::PatternVisitor::apply(GLambdaExpression* stmt, std::list<NodeType>& path) {
  return VisitFlow::SkipCurrent;
}

