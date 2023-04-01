#include "plan/query/PathPlan.h"
#include "base/lang/ASTNode.h"
#include "gqlite.h"
#include "Context.h"

GPathQuery::GPathQuery(GContext* context, GListNode* stmt, gqlite_callback cb, void* cbHandle, const std::string& name)
: GPlan(context->_graph, context->_storage, context->_schedule){
  _scan = new GScanPlan(context, stmt, name);
}

int GPathQuery::prepare() {
  return ECode_Success;
}

int GPathQuery::execute(GVM* gvm, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>& processor) {
  return ECode_Success;
}