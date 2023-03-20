#pragma once
#include "Plan.h"

class GRemoveStmt;
class GRemovePlan: public GPlan {
public:
  GRemovePlan(GContext* context, GRemoveStmt* stmt);
  ~GRemovePlan();
  virtual int execute(GVM* gvm, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>&);

private:
  std::string _group;
  GPlan* _scan;
};