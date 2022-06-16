#include "plan/Plan.h"

GPlan::GPlan(GVirtualNetwork* network, GStorageEngine* store)
: _left(nullptr), _right(nullptr)
, _store(store)
, _network(network) {}

GPlan::~GPlan() {
  if (_left) delete _left;
  if (_right) delete _right;
}