#pragma once

class GVirtualNetwork;
class GPlan {
public:
  GPlan(GVirtualNetwork* network);
  virtual ~GPlan();
  
  virtual int execute() = 0;

private:
  GVirtualNetwork* _network;
  GPlan* _left;
  GPlan* _right;
};
