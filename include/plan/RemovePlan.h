#pragma once
#include "Plan.h"

class GRemoveStmt;
class GRemovePlan: public GPlan {
public:
  GRemovePlan(GVirtualNetwork* network, GStorageEngine* store, GRemoveStmt* stmt);
  ~GRemovePlan();
  virtual int execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>&);

private:
  std::string _group;
  GPlan* _scan;
};