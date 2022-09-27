#include "plan/RemovePlan.h"
#include <stdio.h>
#include "base/lang/RemoveStmt.h"
#include "plan/ScanPlan.h"
#include "StorageEngine.h"

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
  KeyType type = _store->getKeyType(_group);
  switch (type) {
  case KeyType::Integer:
    for (auto itr = keys.begin(), end = keys.end(); itr != end; ++itr)
    {
      uint64_t k = *(uint64_t*)(itr->data());
      _store->del(_group, k);
    }
    break;
  case KeyType::Byte:
    break;
  default:
    break;
  }
  return 0;
}
