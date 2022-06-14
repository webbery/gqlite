#include "plan/Plan.h"

GPlan::GPlan(GVirtualNetwork* network)
: _left(nullptr), _right(nullptr)
, _network(network) {}

GPlan::~GPlan() {
  if (_left) delete _left;
  if (_right) delete _right;
}