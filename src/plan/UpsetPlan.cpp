#include <thread>
#include <future>
#include "plan/UpsetPlan.h"
#include "StorageEngine.h"
#include "VirtualNetwork.h"
#include "gutil.h"

GUpsetPlan::GUpsetPlan(GVirtualNetwork* vn, GStorageEngine* store, GUpsetStmt* ast)
:GPlan(vn, store) {
  UpsetVisitor visitor(*this);
  std::list<NodeType> ln;
  accept(ast->node(), visitor, ln);
}

int GUpsetPlan::prepare() {
  // check graph is create or not.
  auto schema = _store->getSchema();
  if (schema.empty()) return ECode_Fail;
  if (_vertex) {
    _class = MAP_NODE;
  }
  else {
    _class = MAP_EDGE;
  }
  return ECode_Success;
}

int GUpsetPlan::execute(gqlite_callback) {
  if (_store) {
    std::string k = std::get<0>(_key);
    // generate key
    uint32_t uik = unicode32(k);
    _store->write(_class, uik, _value.data(), _value.size());
  }
  return 0;
}
