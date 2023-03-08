#pragma once

#include "executor/Executor.h"

class GFilterExecutor : public GExecutor {
public:
  virtual ~GFilterExecutor();
  virtual int open();
  virtual int execute(GContext* ctx);
  virtual int close();
};