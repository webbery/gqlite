#include "plan/RemovePlan.h"
#include <stdio.h>
#include "base/lang/RemoveStmt.h"
#include "plan/ScanPlan.h"
#include "StorageEngine.h"
#include "gutil.h"

namespace {

  template<typename Key>
  void RemoveEdges(GStorageEngine* store, const std::string& group, const Key& k) {
    bool isFrom = true;
    auto&& relations = store->getRelations(group);
    for (auto& relation : relations) {
      //if (group == std::get<2>(relation)) {
      //  isFrom = false;
      //}
      store->del(std::get<0>(relation), k, isFrom);
    }
  }

}

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


int GRemovePlan::execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value, int status)>& processor)
{
  std::vector<std::string> keys;
  _scan->execute([&keys](KeyType, const std::string& key, const std::string& value, int status) {
    if (status != ECode_Success) return ExecuteStatus::Stop;

    keys.emplace_back(key);
    return ExecuteStatus::Continue;
    });
  if (keys.size() == 0) return ECode_Success;

  KeyType type = _store->getKeyType(_group);
  switch (type) {
  case KeyType::Integer:
    for (auto itr = keys.begin(), end = keys.end(); itr != end; ++itr)
    {
      uint64_t k = *(uint64_t*)(itr->data());
      if (_store->del(_group, k) == ECode_Success) {
        RemoveEdges(_store, _group, k);
      }
    }
    break;
  case KeyType::Byte:
    for (auto itr = keys.begin(), end = keys.end(); itr != end; ++itr)
    {
      if (_store->del(_group, itr->data()) == ECode_Success) {
        RemoveEdges(_store, _group, itr->data());
      }
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
