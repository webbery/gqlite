#pragma once
#include "Plan.h"

class GQueryPlan : public GPlan {
public:
  virtual int prepare();
  virtual int execute(gqlite_callback);
};
