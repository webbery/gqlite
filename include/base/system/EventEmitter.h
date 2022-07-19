#pragma once
#include <any>
#include <map>
#include <functional>
#include "base/parallel/parlay/scheduler.h"

class GEventEmitter {
public:
  GEventEmitter();
  ~GEventEmitter();

  void emit(int, std::any& args);

  void on(int, std::function<void(const std::any&)> f);

  void clear();

private:
  class Job {
  public:
    Job(std::function<void(const std::any&)> f, const std::any& args):_args(args), _f(f) {}
    void operator()() {
      _f(_args);
    }

  private:
    std::function<void(const std::any&)> _f;
    std::any _args;
  };
private:
  std::map<int, std::function<void(const std::any&)>> _listeners;
  parlay::scheduler<Job>* _scheduler;
  std::vector<Job*> _jobs;
};