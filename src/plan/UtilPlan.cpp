#include "plan/UtilPlan.h"
#include "gqlite.h"
#include "VirtualNetwork.h"
#include "StorageEngine.h"
#include "base/lang/AST.h"

GUtilPlan::GUtilPlan(GVirtualNetwork* vn, GStorageEngine* store, GCreateStmt* stmt)
:GPlan(vn, store) {
    _type = UtilType::Creation;
    _var = stmt->name();
    GASTNode* indexes = stmt->indexes();
}
int GUtilPlan::execute() {
  switch (_type)
  {
  case UtilType::Creation:
    if (!_store) return ECode_DISK_OPEN_FAIL;
    return _store->open(std::get<std::string>(_var).c_str());
  default:
    break;
  }
  return ECode_Success;
}
