#include "base/system/EventEmitter.h"

GEventEmitter::GEventEmitter()
:_scheduler(new parlay::scheduler<Job>)
{}

GEventEmitter::~GEventEmitter() {
  delete _scheduler;
  for (Job* job: _jobs) {
    delete job;
  }
}

void GEventEmitter::emit(int event, std::any& args) {
  if (_listeners.count(event)) {
    Job* job = new Job(_listeners[event], args);
    _jobs.push_back(job);
    _scheduler->spawn(job);
  }
}

void GEventEmitter::on(int event, std::function<void(const std::any&)> f) {
  _listeners[event] = f;
}

void GEventEmitter::clear() {
  _listeners.clear();
}