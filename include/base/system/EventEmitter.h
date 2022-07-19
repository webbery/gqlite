#pragma once
#include <any>
#include <map>
#include <functional>
#include <atomic>
#include "base/parallel/parlay/scheduler.h"

class GEventEmitter {
public:
  GEventEmitter();
  ~GEventEmitter();

  void emit(int, std::any& args);

  void on(int, std::function<void(const std::any&)> f);

  void clear();

  void join();

private:
  enum JobStatus : uint8_t {
    Ready,
    Working,
    Finished
  };
  class Job {
  public:
    Job(std::function<void(const std::any&)> f, const std::any& args):_args(args), _f(f), _state(JobStatus::Ready) {}
    void operator()() {
      _state.store(JobStatus::Working);
      _f(_args);
      _state.store(JobStatus::Finished);
    }

    int state() { return _state.load(); }
  private:
    std::atomic_char _state;   /**< 0: ready, 1: working, 2: finished */
    std::function<void(const std::any&)> _f;
    std::any _args;
  };
private:
  std::map<int, std::function<void(const std::any&)>> _listeners;
  parlay::scheduler<Job>* _scheduler;
  std::vector<Job*> _jobs;
};