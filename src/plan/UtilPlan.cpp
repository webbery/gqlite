#include "plan/UtilPlan.h"
#include "gqlite.h"
#include "VirtualNetwork.h"
#include "StorageEngine.h"
#include "base/lang/AST.h"
#include <filesystem>

GUtilPlan::GUtilPlan(GVirtualNetwork* vn, GStorageEngine* store, GCreateStmt* stmt)
:GPlan(vn, store) {
    _type = UtilType::Creation;
    _var = stmt->name();
    GASTNode* indexes = stmt->indexes();
}

GUtilPlan::GUtilPlan(GVirtualNetwork* vn, GStorageEngine* store, GDropStmt* stmt)
:GPlan(vn, store) {
  _type = UtilType::Drop;
  _var = stmt->name();
}

int GUtilPlan::execute(gqlite_callback) {
  switch (_type)
  {
  case UtilType::Creation:
    if (!_store) return ECode_DISK_OPEN_FAIL;
    return _store->open(std::get<std::string>(_var).c_str());
  case UtilType::Drop:
  {
    std::string graph = std::get<std::string>(_var);
    if (std::filesystem::exists(graph)) {
      if (std::remove(graph.c_str())) {
        return ECode_DB_Drop_Fail;
      }
    }
    else {
      return ECode_Graph_Not_Exist;
    }
  }
    break;
  default:
    break;
  }
  return ECode_Success;
}
