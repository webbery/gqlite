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
    for (auto itr = _vertexes.begin(), end = _vertexes.end(); itr != end; ++itr) {
      const auto& key = itr->first;
      int ret = ECode_Fail;
      key.visit(
        [&](std::string k) {
          uint32_t uik = unicode32(k);
          ret = _store->write(_class, uik, itr->second.data(), itr->second.size());
        },
        [&](uint64_t k) {
          ret = _store->write(_class, k, itr->second.data(), itr->second.size());
        });
      if (ret != ECode_Success) return ret;
    }
  }
  return 0;
}
