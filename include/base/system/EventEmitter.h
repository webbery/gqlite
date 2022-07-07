#pragma once
#include "base/parallel/parlay/sequence.h"
#include <any>
#include <map>

class GEventEmitter {
public:
  GEventEmitter();

  void emit(int);
  void emit(int, std::any& args);

  void on(int, std::function<void(const std::any&)> f);
private:
  std::map<int, std::function<void(const std::any&)>> _listeners;
};