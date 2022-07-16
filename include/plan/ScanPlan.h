#pragma once
#include "Plan.h"
#include "Graph/GRAD.h"
#include <string>
#include <atomic>
#include <thread>

class GQueryStmt;
class GScanPlan: public GPlan {
public:
  GScanPlan(GVirtualNetwork* network, GStorageEngine* store, GQueryStmt* stmt);
  virtual int prepare();
  virtual int execute(gqlite_callback);
  virtual int interrupt();

private:
  int scan();
private:
  std::atomic_bool _interrupt;
  /**
   * worker thread will scan graph instance. If it find out a node or
   * edge that match the graph pattern, it will put it to virtual network.
   */
  std::thread _worker;

  GraphPattern _pattern;
  std::string _graph;
};