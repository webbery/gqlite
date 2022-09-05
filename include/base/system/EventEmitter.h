#pragma once
#include <any>
#include <map>
#include <functional>
#include <atomic>
#include "base/parallel/parlay/scheduler.h"

class GEventEmitter {
public:
  GEventEmitter();
  /**
   * @brief Destroy the GEventEmitter object
   *        All task will be delete whatever working state is.
   */
  ~GEventEmitter();

  void emit(int, std::any& args);

  void on(int, std::function<void(const std::any&)> f);

  void clear();

  void join();

  void finish();

  bool is_finish();

private:
  enum JobStatus : uint8_t {
    Ready,
    Working,
    Finished
  };
  class Job {
  public:
    Job(std::atomic_bool& interrupt, std::function<void(const std::any&)> f, const std::any& args)
      :_args(args), _f(f), state(JobStatus::Ready), _intterrupt(interrupt){ _id = ++GEventEmitter::_id; }
    void operator()() {
      if (_intterrupt.load()) return;
      printf("start task: %d\n", _id);
      state.store(JobStatus::Working);
      _f(_args);
      state.store(JobStatus::Finished);
      printf("finish task: %d\n", _id);
    }

    int _id;
    std::atomic_char state;   /**< 0: ready, 1: working, 2: finished */
  private:
    std::function<void(const std::any&)> _f;
    std::any _args;
    std::atomic_bool& _intterrupt;
  };

  static int _id;

  void scavenger();
private:
  std::map<int, std::function<void(const std::any&)>> _listeners;
  parlay::scheduler<Job>* _scheduler;
  std::vector<Job*> _jobs;
  std::atomic_bool _interrupt;
  std::atomic_bool _joinable;
};