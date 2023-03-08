#pragma once

#include <memory>
#include <functional>
#include <map>

typedef void*   fcontext_t;
struct transfer_t {
    fcontext_t  fctx;
    void    *   data;
};

template< std::size_t Max, std::size_t Default, std::size_t Min >
class simple_stack_allocator
{
public:
    static std::size_t maximum_stacksize()
    { return Max; }

    static std::size_t default_stacksize()
    { return Default; }

    static std::size_t minimum_stacksize()
    { return Min; }

    void * allocate( std::size_t size) const
    {
        void * limit = malloc( size);
        if ( ! limit) throw std::bad_alloc();

        return static_cast< char * >( limit) + size;
    }

    void deallocate( void * vp, std::size_t size) const
    {
        void * limit = static_cast< char * >( vp) - size;
        free( limit);
    }
};
using stack_allocator = simple_stack_allocator<8 * 1024 * 1024, 64 * 1024, 8 * 1024>;

class GCoSchedule;
class GCoroutine{
public:
  friend void __coroutine_entry(transfer_t t);
  friend class GCoSchedule;

  enum class Status {
    Uninitialize,
    Ready,
    Suspend,
    Running,
    Finish,
  };

  ~GCoroutine();

  void await();
  void yield();
  void resume();

  Status status() const { return _status; }
  uint8_t id() const { return _id; }
private:
  void init(std::function<void(GCoroutine*)> const& func);

  template <typename F>
  GCoroutine(F func):_status(Status::Uninitialize),_id(0)
  { init(func); }

private:
  std::function<void(GCoroutine*)> _func;

  fcontext_t _ctx{nullptr};

  void* _stack{nullptr};

  GCoSchedule* _schedule;

  Status _status;

  uint8_t _id;
};

class GCoSchedule {
public:
  friend void __coroutine_entry(transfer_t t);
  friend class GCoroutine;

  enum class ScheduleType {
    Main,     /*< bind to main thread */
  };

  GCoSchedule();
  ~GCoSchedule();

  template<typename F>
  GCoroutine* createCoroutine(F f) {
    GCoroutine* c = new GCoroutine(f);
    init(c);
    return c;
  }

  void init(GCoroutine* c);

  void run();

  void join();

  size_t size() const;

private:
  uint8_t _current;
  fcontext_t _main;
  stack_allocator _alloc;
  std::map<uint8_t, GCoroutine*> _coroutines;
};