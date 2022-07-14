#include "base/system/EventEmitter.h"

GEventEmitter::GEventEmitter()
{}

void GEventEmitter::emit(int event, std::any& args) {
  if (_listeners.count(event)) {
    std::function<void()> lambda = [&job = this->_listeners[event], args = args]() {
      job(args);
    };
    // _scheduler.spawn(lambda);
    // _listeners[event](args);
    lambda();
  }
}

void GEventEmitter::on(int event, std::function<void(const std::any&)> f) {
  _listeners[event] = f;
}

void GEventEmitter::clear() {
  _listeners.clear();
}