#include <thread>
#include <future>
#include "plan/UpsetPlan.h"
#include "StorageEngine.h"
#include "VirtualNetwork.h"
#include "gutil.h"
#include <fmt/printf.h>
#include <fmt/color.h>

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

int GUpsetPlan::execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>& processor) {
  if (_store) {
    KeyType type = _store->getKeyType(_class);
    for (auto itr = _vertexes.begin(), end = _vertexes.end(); itr != end; ++itr) {
      const auto& key = itr->first;
      int ret = key.visit(
        [&](std::string k) {
          if (type == KeyType::Integer) {
            fmt::print(fmt::fg(fmt::color::red), "ERROR: upset fail!\nInput key type is string, but require integer\n");
            return ECode_Fail;
          }
          return _store->write(_class, k, itr->second.data(), itr->second.size());
        },
        [&](uint64_t k) {
          if (type == KeyType::Byte) {
            fmt::print(fmt::fg(fmt::color::red), "ERROR: upset fail!\nInput key type is integer, but require string\n");
            return ECode_Fail;
          }
          return _store->write(_class, k, itr->second.data(), itr->second.size());
        });
      if (ret != ECode_Success) return ret;
    }
  }
  return 0;
}
