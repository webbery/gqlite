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
  scan();
#endif
  return ECode_Success;
}

int GScanPlan::interrupt()
{
  _interrupt.store(true);
  _worker.join();
  return ECode_Success;
}

int GScanPlan::scan()
{
  for (std::string& group : _queries)
  {
    auto cs = _store->getCursor(group);
    auto data = cs.to_first(false);
    while (data)
    {
      std::string str((char*)data.value.byte_ptr(), data.value.size());
      //printf("scan: %s\n", str.c_str());
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
