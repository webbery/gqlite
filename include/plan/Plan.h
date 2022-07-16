#pragma once
#include "gqlite.h"

class GVirtualNetwork;
class GStorageEngine;
typedef int (*gqlite_callback)(_gqlite_result*);
class GPlan {
public:
  GPlan(GVirtualNetwork* network, GStorageEngine* store);
  virtual ~GPlan();
  
  /** After Plan created, before Plan execute,
   * the prepare function will be called for validating preliminary.
   * For example: before update execute, we should check database created or not.
   */
  virtual int prepare() { return 0; }
  virtual int execute(gqlite_callback) = 0;
  /**
   * Try to interrupt plan when it still working
   */
  virtual int interrupt() { return 0; }
  void addLeft(GPlan* plan) { _left = plan; }

  inline GPlan* left() { return _left; }
protected:
  GVirtualNetwork* _network;
  GStorageEngine* _store;
  GPlan* _left;
  GPlan* _right;
};
