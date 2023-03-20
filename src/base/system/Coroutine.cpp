#include "base/system/Coroutine.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include "schedule/DefaultSchedule.h"

#if __SANITIZE_ADDRESS__
#include <sanitizer/asan_interface.h>
extern "C" {
  void __sanitizer_start_switch_fiber(void** fake_stack_save, const void* stack_bottom, size_t stack_size);
  void __sanitizer_finish_switch_fiber(void* fake_stack_save, const void** stack_bottom_old, size_t* stack_size_old);
}
#else
static inline void __sanitizer_start_switch_fiber(...) { }
static inline void __sanitizer_finish_switch_fiber(...) { }
#endif

extern "C" transfer_t  jump_fcontext( fcontext_t const to, void * vp);
extern "C" fcontext_t  make_fcontext( void * sp, std::size_t size, void (* fn)( transfer_t) );

void __coroutine_entry(transfer_t t) {
  GDefaultSchedule* schedule = (GDefaultSchedule*)t.data;
  int id = schedule->_current;
  schedule->_main = t.fctx;
  GCoroutine* c = schedule->_coroutines[id];
  c->_func(c);
  c->_status = GCoroutine::Status::Finish;
  jump_fcontext(t.fctx, t.data);
}

void GCoroutine::init(std::function<void(GCoroutine*)> const& func) {
  _func = func;
}

GCoroutine::~GCoroutine() {
  if (_stack) _schedule->_alloc.deallocate(_stack, stack_allocator::default_stacksize());
}

void GCoroutine::resume() {
  switch (_status) {
    case Status::Ready: {
      _schedule->_current = _id;
      _status = Status::Running;

      _stack = _schedule->_alloc.allocate( stack_allocator::default_stacksize());
      _ctx = make_fcontext(_stack, _schedule->_alloc.default_stacksize(), __coroutine_entry);
      transfer_t t = jump_fcontext(_ctx, this->_schedule);
      _ctx = t.fctx;
    }
    break;
    case Status::Suspend: {
      _schedule->_current = _id;
      _status = Status::Running;
      transfer_t t = jump_fcontext(_ctx, this->_schedule);
      _ctx = t.fctx;
      assert(_status != Status::Running);
    }
    break;
    default:
    break;
  }
}

void GCoroutine::yield() {
  if (_status == Status::Running) {
    _status = Status::Suspend;
    transfer_t t = jump_fcontext(_schedule->_main, nullptr);
    _ctx = t.fctx;
  }
}

