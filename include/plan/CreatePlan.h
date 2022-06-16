#pragma once
#include <string>
#include <vector>
#include "Plan.h"

struct GASTNode;
class GCreatePlan: public GPlan {
public:
  GCreatePlan(GVirtualNetwork*, GStorageEngine* store, GASTNode*);
  ~GCreatePlan() {}

  int execute();

private:
  std::vector<std::string> _indexes;
};