#pragma once
#include "Plan.h"

class GScanPlan: public GPlan {
public:
  virtual int execute();
};