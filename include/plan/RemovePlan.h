#pragma once
#include "Plan.h"

class GRemoveStmt;
class GRemovePlan: public GPlan {
public:
  GRemovePlan(GVirtualNetwork* network, GStorageEngine* store, GRemoveStmt* stmt);
  ~GRemovePlan();
  virtual int execute(gqlite_callback);

private:
  std::string _group;
  GPlan* _scan;
};