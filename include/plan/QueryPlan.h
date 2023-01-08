#pragma once
#include "Plan.h"

class GScanPlan;
class GQueryStmt;
class GQueryPlan : public GPlan {
public:
  GQueryPlan(std::map<std::string, GVirtualNetwork*>& network, GStorageEngine* store, GQueryStmt* stmt, gqlite_callback cb, void* cbHandle);
  ~GQueryPlan();
  virtual int prepare();
  virtual int execute(const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>&);

private:
  void convert_vertex(KeyType type, const std::string& key, nlohmann::json& value, gqlite_result& result);
  void convert_edge(const std::string& key, nlohmann::json& value, gqlite_result& result);
  // convert obj to display type
  void beautify(nlohmann::json& input);

private:
  GScanPlan* _scan;
  gqlite_callback _cb;
  void* _handle;
};
