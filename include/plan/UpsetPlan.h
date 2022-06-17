#pragma once
#include "Plan.h"

struct GASTNode;
class GUpsetPlan: public GPlan {
public:
  GUpsetPlan(GVirtualNetwork* vn, GStorageEngine* store, GASTNode* ast);
  virtual int execute();
};