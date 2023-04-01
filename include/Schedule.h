#pragma once
#include "base/system/Coroutine.h"

template <typename T> class Future;
class GCoroutine;
class GSchedule {
public:
  enum class ScheduleInstanceType {
    Main,     /*< bind to main thread */
  };

  virtual ~GSchedule() {}

  virtual Future<int>&& schedule() = 0;

  virtual bool init(GWorker* worker) = 0;

  template<typename F>
  GCoroutine* addCoroutine(F f) {
    GCoroutine* c = new GCoroutine(f);
    init(c);
    return c;
  }

protected:
  
};
