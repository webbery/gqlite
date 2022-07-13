#pragma once

class GVirtualNetwork;
class GStorageEngine;
class GPlan {
public:
  GPlan(GVirtualNetwork* network, GStorageEngine* store);
  virtual ~GPlan();
  
  /** After Plan created, before Plan execute,
   * the prepare function will be called for validating preliminary.
   * For example: before update execute, we should check database created or not.
   */
  virtual int prepare() { return 0; }
  virtual int execute() = 0;
  void addLeft(GPlan* plan) { _left = plan; }

  inline GPlan* left() { return _left; }
protected:
  GVirtualNetwork* _network;
  GStorageEngine* _store;
  GPlan* _left;
  GPlan* _right;
};
