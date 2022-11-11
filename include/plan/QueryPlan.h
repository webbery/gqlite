#pragma once
#include "Plan.h"

class GScanPlan;
class GQueryStmt;
class GQueryPlan : public GPlan {
public:
  GQueryPlan(std::map<std::string, GVirtualNetwork*>& network, GStorageEngine* store, GQueryStmt* stmt, gqlite_callback cb, void* cbHandle);
  ~GQueryPlan();
  virtual int prepare();
  virtual int execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>&);
private:
  GScanPlan* _scan;
  gqlite_callback _cb;
  void* _handle;
};
