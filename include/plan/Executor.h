#pragma once
#include <future>

struct GContext;
struct GPlanNode;

class GExecuteResult {};

/**
 * base class
*/
class GExecutor {
public:
  static GExecutor* create(GPlanNode* planNode);

  virtual ~GExecutor();

  GExecutor(const GExecutor&) = delete;
  GExecutor& operator = (const GExecutor&) = delete;

  virtual int open() = 0;
  virtual std::future<GExecuteResult> execute(GContext* ctx) = 0;
  virtual int close() = 0;

private:
  GExecutor();
  
};