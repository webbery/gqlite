#include "base/system/Coroutine.h"
#include <cstdlib>

#ifdef WIN32
void __stdcall __win_entry(LPVOID lpParameter) {
  GCoSchedule* schedule = (GCoSchedule*)lpParameter;
  GCoroutine* c = schedule->_coroutines[schedule->_current];
  c->_func(c);
  c->_status = GCoroutine::Status::Finish;
  SwitchToFiber(schedule->_main);
}
#else
void __unix_entry(uint32_t l32, uint32_t h32) {
  printf("entry()\n");
  uintptr_t ptr = (uintptr_t)l32 | ((uintptr_t)h32 << 32);
  GCoSchedule* schedule = (GCoSchedule*)ptr;
  int id = schedule->_current;
  GCoroutine* c = schedule->_coroutines[id];
  c->_func(c);
  c->_status = GCoroutine::Status::Finish;
}

static void __save_stack(GCoroutine* c, char* top) {

}
#endif

void GCoroutine::init(std::function<void(GCoroutine*)> const& func) {
  _func = func;
}

GCoroutine::~GCoroutine() {
#ifdef WIN32
  DeleteFiber(_context);
#else
#endif
}

void GCoroutine::resume() {
  switch (_status) {
    case Status::Await:
    case Status::Ready: {
      _schedule->_current = _id;
      _status = Status::Running;
#ifdef WIN32
      SwitchToFiber(_context);
#else
      getcontext(&_context);
      _context.uc_stack.ss_sp = _schedule->_stack;
      _context.uc_stack.ss_size = STACK_SIZE;
      _context.uc_link = &_schedule->_main;
      uintptr_t ptr = (uintptr_t)_schedule;
      makecontext(&_context,(void (*)(void)) __unix_entry, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
      swapcontext(&_schedule->_main, &_context);
#endif
    }
    break;
    case Status::Suspend: {
      _schedule->_current = _id;

    }
    break;
    default:
    break;
  }
}

void GCoroutine::yield() {
  auto id = _schedule->_current;
#ifdef WIN32
#else
  __save_stack(this, _schedule->_stack + STACK_SIZE);
  _status = Status::Suspend;
  _schedule->_current = 0;
  swapcontext(&_context, &_schedule->_main);
#endif
}

GCoSchedule::GCoSchedule():_current(0) {
#ifdef WIN32
  _main = ConvertThreadToFiber(NULL);
#else
  _stack = (char*)malloc(STACK_SIZE);
#endif
}

GCoSchedule::~GCoSchedule() {
#ifdef WIN32
#else
  free(_stack);
#endif
}

void GCoSchedule::join() {
  printf("join()\n");
}

void GCoSchedule::run() {
  while (!_coroutines.empty()) {
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
  _coroutines.clear();
  printf("finish\n");
}

void GCoSchedule::init(GCoroutine* c) {
  uint8_t id = 1;
  if (_coroutines.size()) {
    id = _coroutines.rbegin()->first + 1;
  }
  while (_coroutines.find(id) != _coroutines.end()) {
    id += 1;
  }
  c->_id = id;
#ifdef WIN32
  c->_context = CreateFiber(0, __win_entry, this);
#endif
  c->_status = GCoroutine::Status::Ready;
  c->_schedule = this;
  _coroutines[id] = c;
}

