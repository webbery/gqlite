#include "plan/QueryPlan.h"
#include "plan/ScanPlan.h"
#include "StorageEngine.h"

GQueryPlan::GQueryPlan(GVirtualNetwork* network, GStorageEngine* store, GQueryStmt* stmt, gqlite_callback cb)
  :GPlan(network, store)
  , _cb(cb)
{
  _scan = new GScanPlan(network, store, stmt);
}

GQueryPlan::~GQueryPlan()
{
  delete _scan;
}

int GQueryPlan::prepare()
{
  return 0;
}

int GQueryPlan::execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>& processor)
{
  if (_cb) {
    _scan->execute([&cb = this->_cb](KeyType type, const std::string& key, const std::string& value) {
      gqlite_result result;
      result.count = 1;
      result.type = gqlite_result_type_node;
      result.errcode = ECode_Success;
      result.nodes = new gqlite_node;
      result.nodes->_type = gqlite_node_type::gqlite_node_type_vertex;
      result.nodes->_vertex = new gqlite_vertex;
      if (type == KeyType::Integer) {
        result.nodes->_vertex->uid = atoll((char*)key.data());
      }
      else {
        result.nodes->_vertex->cid = (char*)key.data();
      }
      size_t len = value.size();
      result.nodes->_vertex->properties = new char[len];
      result.nodes->_next = nullptr;
      memcpy(result.nodes->_vertex->properties, value.data(), sizeof(char) * len);
      cb(&result);
      delete[] result.nodes->_vertex->properties;
      delete result.nodes->_vertex;
      delete result.nodes;
      return ExecuteStatus::Continue;
    });
    
  }
  return 0;
}
