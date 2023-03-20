#pragma once
#include "VirtualNetwork.h"
#include "base/lang/ASTNode.h"
#include "base/lang/WalkDeclaration.h"
#include "plan/ScanPlan.h"

class GPathQuery: public GPlan {
public:
  GPathQuery(GContext* context, GListNode* stmt, gqlite_callback cb, void* cbHandle, const std::string& name);

  // construct start and end node
  virtual int prepare();

  virtual int execute(GVM* gvm, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>& processor);
private:
  void setStart();
  void setEnd();

  void buildGraph();


  GScanPlan* _scan{nullptr};
};