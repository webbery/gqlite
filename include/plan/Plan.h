#pragma once

class GVirtualNetwork;
class GStorageEngine;
class GPlan {
public:
  GPlan(GVirtualNetwork* network, GStorageEngine* store);
  virtual ~GPlan();
  
  virtual int execute() = 0;

protected:
  GVirtualNetwork* _network;
  GStorageEngine* _store;
  GPlan* _left;
  GPlan* _right;
};
