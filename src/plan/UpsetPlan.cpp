#include <thread>
#include <future>
#include "plan/UpsetPlan.h"
#include "StorageEngine.h"
#include "VirtualNetwork.h"

GUpsetPlan::GUpsetPlan(GVirtualNetwork* vn, GStorageEngine* store, GUpsetStmt* ast)
:GPlan(vn, store)
,_class(SCHEMA_CLASS_DEFAULT) {
  UpsetVisitor visitor(*this);
  std::list<NodeType> ln;
  accept(ast->node(), visitor, ln);
}

int GUpsetPlan::prepare() {
  // check graph is create or not.
  auto schema = _store->getSchema();
  if (schema.empty()) return ECode_Fail;
  return ECode_Success;
}

int GUpsetPlan::execute() {
  printf("upset plan, class: %s, key: %s\n", _class.c_str(), std::get<0>(_key).c_str());
  if (_store) {
    std::string k = std::get<0>(_key);
    _store->write(_class, k, _value.data(), _value.size());
  }
  return 0;
}
