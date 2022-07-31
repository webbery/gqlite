#pragma once
#include "Plan.h"

class GRemovePlan: public GPlan {
public:
  virtual int execute(gqlite_callback);
};