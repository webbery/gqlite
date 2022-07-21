#include <thread>
#include <future>
#include "plan/UpsetPlan.h"
#include "StorageEngine.h"
#include "VirtualNetwork.h"
#include "gutil.h"

GUpsetPlan::GUpsetPlan(GVirtualNetwork* vn, GStorageEngine* store, GUpsetStmt* ast)
:GPlan(vn, store)
,_class(ast->name()) {
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

int GUpsetPlan::execute(gqlite_callback) {
  if (_store) {
    uint32_t uik = 0;
    switch (_key.index()) {
    case 0:
    {
      std::string k = std::get<0>(_key);
      uik = unicode32(k);
    }
      break;
    case 1:
      uik = std::get<1>(_key);
      break;
    }
    // generate key
    _store->write(_class, uik, _value.data(), _value.size());
  }
  return 0;
}
