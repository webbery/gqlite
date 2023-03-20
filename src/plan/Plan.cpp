#include "plan/Plan.h"
#include "base/gvm/Chunk.h"
#include "base/gvm/Compiler.h"

void init_result_nodes(gqlite_result& result)
{

}

GPlan::GPlan(GVirtualNetwork* network, GStorageEngine* store, GDefaultSchedule* schedule)
: _left(nullptr), _right(nullptr)
, _compiler(nullptr)
, _store(store)
, _schedule(schedule) {
  // make network is the first one
  if (network) _network["\1"] = network;
}

GPlan::~GPlan() {
  if (_left) delete _left;
  if (_right) delete _right;
  if (_compiler) delete _compiler;
}