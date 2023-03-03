#pragma once
#include "Plan.h"

class GRemoveStmt;
class GRemovePlan: public GPlan {
public:
  GRemovePlan(std::map<std::string, GVirtualNetwork*>& network, GStorageEngine* store, GRemoveStmt* stmt, GCoSchedule* schedule);
  ~GRemovePlan();
  virtual int execute(GVM* gvm, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>&);

private:
  std::string _group;
  GPlan* _scan;
};