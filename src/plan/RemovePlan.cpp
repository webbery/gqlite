#include "plan/RemovePlan.h"
#include <stdio.h>
#include "base/lang/RemoveStmt.h"
#include "plan/ScanPlan.h"
#include "StorageEngine.h"
#include "gutil.h"

GRemovePlan::GRemovePlan(std::map<std::string, GVirtualNetwork*>& networks, GStorageEngine* store, GRemoveStmt* stmt)
  :GPlan(networks, store)
{
  _group = stmt->name();
  _scan = new GScanPlan(networks, _store, stmt->node(), _group);
}

GRemovePlan::~GRemovePlan()
{
  delete _scan;
}

int GRemovePlan::execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>& processor)
{
  std::vector<std::string> keys;
  _scan->execute([&keys](KeyType, const std::string& key, const std::string& value) {
    keys.emplace_back(key);
    return ExecuteStatus::Continue;
    });
  if (keys.size() == 0) return ECode_Success;

  KeyType type = _store->getKeyType(_group);
  if (type == KeyType::Integer || type == KeyType::Byte) {
    // try find their relation data and then remove
    std::list<std::string>&& relations = _store->getRelations(_group);
  }
  switch (type) {
  case KeyType::Integer:
    for (auto itr = keys.begin(), end = keys.end(); itr != end; ++itr)
    {
      uint64_t k = *(uint64_t*)(itr->data());
      _store->del(_group, k);
    }
    break;
  case KeyType::Byte:
    for (auto itr = keys.begin(), end = keys.end(); itr != end; ++itr)
    {
      _store->del(_group, itr->data());
    }
    break;
  case KeyType::Edge:
    for (auto itr = keys.begin(), end = keys.end(); itr != end; ++itr)
    {
      gql::edge_id eid = gql::to_edge_id(*itr);
      _store->del(_group, *itr);
      gql::release_edge_id(eid);
    }
    break;
  default:
    return ECode_Remove_Unknow_Type;
  }
  return 0;
}
