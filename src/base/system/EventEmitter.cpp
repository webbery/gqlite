#include "base/system/EventEmitter.h"
#include <chrono>
#include <thread>
//
//int GEventEmitter::_id = 0;
//
//GEventEmitter::GEventEmitter()
//:_scheduler(new parlay::scheduler<Job>)
//, _interrupt(false)
//, _joinable(true)
//{}
//
//GEventEmitter::~GEventEmitter() {
//  if (_scheduler) {
//    delete _scheduler;
//    _scheduler = nullptr;
//  }
//  for (auto job : _jobs) {
//    delete job;
//  }
//  _jobs.clear();
//}
//
//void GEventEmitter::emit(int event, std::any& args) {
//  if (_interrupt.load()) return;
//  if (_listeners.count(event)) {
//    Job* job = new Job(_interrupt, _listeners[event], args);
//    _jobs.push_back(job);
//#if defined(_PRINT_FORMAT_)
//    printf("emit::add job %d\n", job->_id);
//#endif
//    _scheduler->spawn(job);
//  }
//  if (_joinable.load()) scavenger();
//}
//
//void GEventEmitter::on(int event, std::function<void(const std::any&)> f) {
//  _listeners[event] = f;
//}
//
//void GEventEmitter::clear() {
//  _listeners.clear();
//}
//
//void GEventEmitter::join() {
//  _joinable.store(false);
//  while (_jobs.size()) {
//    Job* job = _jobs.front();
//#if defined(_PRINT_FORMAT_)
//    printf("before delete job\n");
//#endif
//    _scheduler->wait([job, &jobs = this->_jobs]() {
//      if (job->state.load() == JobStatus::Finished) {
//        jobs.erase(jobs.begin());
//#if defined(_PRINT_FORMAT_)
//        printf("join::delete job %d\n", job->_id);
//#endif
//        delete job;
//        return true;
//      }
//      return false;
//    }, true);
//  }
////#if defined(_PRINT_FORMAT_)
////  printf("before delete scheduler\n");
////#endif
//}
//
//void GEventEmitter::finish()
//{
//  _interrupt.store(true);
//  _scheduler->finish();
//}
//
//bool GEventEmitter::is_finish()
//{
//  return _interrupt;
//}
//
//void GEventEmitter::scavenger()
//{
//  for (auto itr = _jobs.begin(); itr != _jobs.end(); ) {
//    if ((*itr)->state.load() == JobStatus::Finished) {
//#if defined(_PRINT_FORMAT_)
//      printf("scavenger delete job %d\n", (*itr)->_id);
//#endif
//      delete* itr;
//      itr = _jobs.erase(itr);
//    }
//    else {
//      ++itr;
//    }
//  }
//}
