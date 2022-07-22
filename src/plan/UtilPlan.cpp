#include "plan/UtilPlan.h"
#include "gqlite.h"
#include "VirtualNetwork.h"
#include "StorageEngine.h"
#include "base/lang/AST.h"
#include <filesystem>

#define CHECK_RETURN(expr) {\
  int ret = (expr);\
  if (ret != ECode_Success) return ret;\
}

GUtilPlan::GUtilPlan(GVirtualNetwork* vn, GStorageEngine* store, GCreateStmt* stmt)
:GPlan(vn, store) {
    _type = UtilType::Creation;
    _var = stmt->name();
    GASTNode* groups = stmt->groups();
    if (groups) {
      GArrayExpression* array = (GArrayExpression*)groups->_value;
      for (auto item: *array) {
        GGroupStmt* stmt = reinterpret_cast<GGroupStmt*>(item->_value);
        std::string name = stmt->name();
        GASTNode* node = stmt->properties();
        if (node) {
          std::vector<std::string> props;
          GArrayExpression* array = reinterpret_cast<GArrayExpression*>(node->_value);
          for (auto prop: *array) {
            props.emplace_back(GetString(prop));
          }
          _vParams2.emplace_back(props);
        }
        _vParams1.emplace_back(name);
      }
    }
    GASTNode* indexes = stmt->indexes();
    if (indexes) {}
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
    StoreOption opt;
    opt.compress = 1;
    CHECK_RETURN(_store->open(std::get<std::string>(_var).c_str(), opt));
    for (auto item: _vParams1) {
      MapInfo info = {0};
      info.key_type = 0;
      info.value_type = ClassType::String;
      _store->addMap(std::get<std::string>(item), info);
    }
    break;
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
