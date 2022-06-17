#pragma once

class GVirtualNetwork;
class GStorageEngine;
class GPlan {
public:
  GPlan(GVirtualNetwork* network, GStorageEngine* store);
  virtual ~GPlan();
  
  virtual int execute() = 0;
  void addLeft(GPlan* plan) { _left = plan; }

  inline GPlan* left() { return _left; }
protected:
  GVirtualNetwork* _network;
  GStorageEngine* _store;
  GPlan* _left;
  GPlan* _right;
};
