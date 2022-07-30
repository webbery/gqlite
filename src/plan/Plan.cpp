#include "plan/Plan.h"

void init_result_nodes(gqlite_result& result)
{

}

GPlan::GPlan(GVirtualNetwork* network, GStorageEngine* store)
: _left(nullptr), _right(nullptr)
, _store(store)
, _network(network) {}

GPlan::~GPlan() {
  if (_left) delete _left;
  if (_right) delete _right;
}