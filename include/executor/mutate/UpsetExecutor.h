#pragma once

#include "executor/Executor.h"

class GUpsetExecutor : public GExecutor {
public:
  virtual ~GUpsetExecutor();
  virtual int open();
  virtual std::future<GExecuteResult> execute(GContext* ctx);
  virtual int close();
};