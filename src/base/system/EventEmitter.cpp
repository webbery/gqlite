#include "base/system/EventEmitter.h"
#include <iostream>
#include <chrono>
#include <thread>

GEventEmitter::GEventEmitter()
:_scheduler(new parlay::scheduler<Job>)
{}

GEventEmitter::~GEventEmitter() {
  join();
  for (Job* job: _jobs) {
    while(job->state() == JobStatus::Working) continue;
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

void GEventEmitter::join() {
  if (_scheduler) {
    delete _scheduler;
    _scheduler = nullptr;
  }
}