#pragma once
#include <any>
#include <map>
#include <functional>
#include "base/parallel/parlay/scheduler.h"

class GEventEmitter {
public:
  GEventEmitter();

  void emit(int, std::any& args);

  void on(int, std::function<void(const std::any&)> f);

  void clear();
private:
  std::map<int, std::function<void(const std::any&)>> _listeners;
  parlay::scheduler<std::function<void()>> _scheduler;
};