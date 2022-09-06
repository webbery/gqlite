#include "base/system/EventEmitter.h"
#include <iostream>
#include <chrono>
#include <thread>

int GEventEmitter::_id = 0;

GEventEmitter::GEventEmitter()
:_scheduler(new parlay::scheduler<Job>)
, _interrupt(false)
, _joinable(true)
{}

GEventEmitter::~GEventEmitter() {
  if (_scheduler) {
    delete _scheduler;
    _scheduler = nullptr;
  }
  for (auto job : _jobs) {
    delete job;
  }
  _jobs.clear();
}

void GEventEmitter::emit(int event, std::any& args) {
  if (_interrupt.load()) return;
  if (_listeners.count(event)) {
    Job* job = new Job(_interrupt, _listeners[event], args);
    _jobs.push_back(job);
    printf("emit::add job %d\n", job->_id);
    _scheduler->spawn(job);
  }
  if (_joinable.load()) scavenger();
}

void GEventEmitter::on(int event, std::function<void(const std::any&)> f) {
  _listeners[event] = f;
}

void GEventEmitter::clear() {
  _listeners.clear();
}

void GEventEmitter::join() {
  _joinable.store(false);
  while (_jobs.size()) {
    Job* job = _jobs.front();
    printf("before delete job\n");
    _scheduler->wait([job, &jobs = this->_jobs]() {
      if (job->state.load() == JobStatus::Finished) {
        jobs.erase(jobs.begin());
        printf("join::delete job %d\n", job->_id);
        delete job;
        return true;
      }
      return false;
    }, true);
  }
  printf("before delete scheduler\n");
  if (_scheduler) {
    delete _scheduler;
    _scheduler = nullptr;
  }
}

void GEventEmitter::finish()
{
  _interrupt.store(true);
  _scheduler->finish();
}

bool GEventEmitter::is_finish()
{
  return _interrupt;
}

void GEventEmitter::scavenger()
{
  for (auto itr = _jobs.begin(); itr != _jobs.end(); ) {
    if ((*itr)->state.load() == JobStatus::Finished) {
      printf("scavenger delete job %d\n", (*itr)->_id);
      delete* itr;
      itr = _jobs.erase(itr);
    }
    else {
      ++itr;
    }
  }
}
