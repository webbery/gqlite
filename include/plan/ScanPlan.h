#pragma once
#include "Plan.h"
#include "Graph/GRAD.h"

class GScanPlan: public GPlan {
public:
  virtual int execute();

private:
  GraphPattern _pattern;
};