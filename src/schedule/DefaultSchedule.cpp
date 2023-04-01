#include "schedule/DefaultSchedule.h"
#include <cassert>
#include "base/Promise.h"
#include "base/system/Coroutine.h"
#include "base/Future.h"

GDefaultSchedule::GDefaultSchedule(GContext* context)
  :_current(0), _maxID(0) {

}

GDefaultSchedule::~GDefaultSchedule() {
  for (auto itr = _coroutines.begin(); itr != _coroutines.end(); ++itr) {
    delete itr->second;
  }
  _coroutines.clear();
  for (auto itr = _cores.begin(); itr != _cores.end(); ++itr) {
    for (gql::Executor* executor : (*itr)->_listeners) {
      delete executor;
    }
    delete (*itr);
  }
  _cores.clear();
}

Future<int>&& GDefaultSchedule::schedule() {
  auto pi = async([]() {return 1; });
  return std::move(pi.getFuture());
}

int GDefaultSchedule::buildPlanGraph(GPlaneNode* root, GPlaneNode* tail) {
  return 0;
}

void GDefaultSchedule::join() {
}

void GDefaultSchedule::run() {
  while (!_coroutines.empty() && _cores.empty()) {
    for (auto itr = _coroutines.begin(); itr != _coroutines.end(); ) {
      if (itr->second->status() == GCoroutine::Status::Finish) {
        delete itr->second;
        itr = _coroutines.erase(itr);
      }
      else {
        itr->second->resume();
        ++itr;
      }
    }
  }
}

bool GDefaultSchedule::init(GWorker* w) {
  uint8_t id = ++_maxID;
  while (_coroutines.find(id) != _coroutines.end()) {
    id += 1;
  }
  GCoroutine* c = static_cast<GCoroutine*>(w);
  c->_id = id;
  c->_status = GCoroutine::Status::Ready;
  c->_schedule = this;
  _coroutines[id] = c;
  return true;
}
