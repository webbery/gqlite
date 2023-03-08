#include "plan/PathPlan.h"
#include "base/lang/ASTNode.h"
#include "gqlite.h"

GPathQuery::GPathQuery(std::map<std::string, GVirtualNetwork*>& network, GStorageEngine* store, GListNode* stmt,
  GCoSchedule* schedule, gqlite_callback cb, void* cbHandle, const std::string& name)
: GPlan(network, store, schedule){
  _scan = new GScanPlan(network, store, stmt, schedule, name);
}

int GPathQuery::prepare() {
  return ECode_Success;
}

int GPathQuery::execute(GVM* gvm, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>& processor) {
  return ECode_Success;
}