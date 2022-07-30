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
  parseQuery(query);
}

int GScanPlan::prepare()
{
  if (_graph.empty()) return ECode_Graph_Not_Exist;
  if (!std::filesystem::exists(_graph)) {
    return ECode_Graph_Not_Exist;
  }
  return ECode_Success;
}

int GScanPlan::execute(gqlite_callback cb) {
  _interrupt.store(false);
#ifdef GQLITE_MULTI_THREAD
  _worker = std::thread(&GScanPlan::scan, this);
#else
  scan(cb);
#endif
  return ECode_Success;
}

int GScanPlan::interrupt()
{
  _interrupt.store(true);
  _worker.join();
  return ECode_Success;
}

int GScanPlan::scan(gqlite_callback cb)
{
  for (std::string& group : _queries)
  {
    auto cs = _store->getCursor(group);
    auto data = cs.to_first(false);
    while (data)
    {
      std::string str((char*)data.value.byte_ptr(), data.value.size());
      //printf("scan: %s\n", str.c_str());
      switch (_queryType)
      {
      case GScanPlan::QueryType::SimpleScan:
        if (cb) {
          gqlite_result result;
          result.count = 1;
          result.type = gqlite_result_type_node;
          result.errcode = ECode_Success;
          result.nodes = new gqlite_node;
          result.nodes->_type = gqlite_node_type::gqlite_node_type_vertex;
          result.nodes->_vertex = new gqlite_vertex;
          result.nodes->_vertex->uid = atoi((char*)data.key.byte_ptr());
          size_t len = data.value.size();
          result.nodes->_vertex->properties = new char[len];
          memcpy(result.nodes->_vertex->properties, data.value.byte_ptr(), sizeof(char) * len);
          cb(&result);
          delete result.nodes->_vertex->properties;
          delete result.nodes->_vertex;
          delete result.nodes;
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

void GScanPlan::parseQuery(GASTNode* query)
{
  if (query->_nodetype == NodeType::ArrayExpression) {
    GArrayExpression* arr = reinterpret_cast<GArrayExpression*>(query->_value);
    for (auto itr = arr->begin(), end = arr->end(); itr != end; ++itr) {
      switch ((*itr)->_nodetype)
      {
      case NodeType::MemberExpression:
        break;
      case NodeType::Literal:
        _queries.emplace_back(GetString(query));
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
