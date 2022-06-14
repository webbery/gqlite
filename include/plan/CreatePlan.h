#pragma once
#include "Plan.h"

class GCreatePlan: public GPlan {
public:
  GCreatePlan(GVirtualNetwork*);
  ~GCreatePlan() {}

  int execute();
};