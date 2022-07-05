#include "base/system/EventEmitter.h"

GEventEmitter::GEventEmitter() {}

void GEventEmitter::emit(int event, std::any& args) {
  if (_listeners.count(event)) {
    _listeners[event](args);
  }
}

void GEventEmitter::on(int event, std::function<void(const std::any&)> f) {
  _listeners[event] = f;
}