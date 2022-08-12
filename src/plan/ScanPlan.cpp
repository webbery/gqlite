#include "plan/ScanPlan.h"
#include "base/lang/ASTNode.h"
#include "base/lang/QueryStmt.h"
#include "gqlite.h"
#include "StorageEngine.h"
#include <filesystem>

GScanPlan::GScanPlan(GVirtualNetwork* network, GStorageEngine* store, GQueryStmt* stmt)
:GPlan(network, store)
, _queryType(QueryType::SimpleScan)
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

GScanPlan::GScanPlan(GVirtualNetwork* network, GStorageEngine* store, GASTNode* condition, const std::string& group)
  :GPlan(network, store)
  , _queryType(QueryType::SimpleScan)
{
  auto jsn = store->getSchema();
  _graph = jsn[SCHEMA_GRAPH_NAME];

  _queries.push_back(group);
  parseConditions(condition);
}

GScanPlan::~GScanPlan()
{
  for (EntityNode* node : _pattern._nodes) {
    delete node;
  }
  _pattern._nodes.clear();
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

int GScanPlan::scan(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>& cb)
{
  for (std::string& group : _queries)
  {
    auto cs = _store->getCursor(group);
    KeyType type = _store->getKeyType(group);
    auto data = cs.to_first(false);
    while (data)
    {
      std::string str((char*)data.value.byte_ptr(), data.value.size());
      //printf("scan: %s\n", str.c_str());
      switch (_queryType)
      {
      case GScanPlan::QueryType::SimpleScan:
      {
        key_t vKey;
        if (type == KeyType::Integer) {
          auto v = *(uint64_t*)data.key.byte_ptr();
          vKey = (uint64_t)v;
        }
        else {
          std::string key((char*)data.key.byte_ptr(), data.key.size());
          vKey = key;
        }
        bool result = true;
        for (auto& pred : _pattern._node_predicates[(long)LogicalPredicate::And]) {
          result &= pred.visit([&vKey](std::function<bool(const key_t&)> op) {
            return op(vKey);
            },
            [](std::function<bool(const attribute_t&)> op) {
              //return op(vKey);
              return true;
            });
          if (!result) ECode_Success;
        }
        for (auto& pred: _pattern._node_predicates[(long)LogicalPredicate::Or])
        {
        }
        if (result) {
          cb(type, std::string((char*)data.key.byte_ptr(), data.key.size()), str);
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
      data = cs.to_next(false);
    }
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
  //_pattern._opl = [](const Variant<std::string,uint64_t>&) { return true; };
  if (conditions == nullptr) return;
  PatternVisitor visitor(_pattern);
  std::list<NodeType> lNodes;
  accept(conditions, visitor, lNodes);
  if (_pattern._edges.size() != 0) _queryType = QueryType::Match;
}

VisitFlow GScanPlan::PatternVisitor::apply(GVertexDeclaration* stmt, std::list<NodeType>& path)
{
  if (!stmt->vertex()) {
    // equal id
    std::string key = GetString(stmt->key());
    predicate_t pred = static_cast<std::function<bool(const key_t&)>>([key](const Variant<std::string, uint64_t>& input)->bool {
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
    EntityNode* node = new EntityNode;
    VertexJsonVisitor visitor(_pattern, node);
    std::list<NodeType> lNodes;
    accept(stmt->vertex(), visitor, lNodes);
    //node->_attrs.push_back()
    _pattern._nodes.push_back(node);

  }
  return VisitFlow::Children;
}

VisitFlow GScanPlan::PatternVisitor::apply(GLiteral* stmt, std::list<NodeType>& path)
{
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
