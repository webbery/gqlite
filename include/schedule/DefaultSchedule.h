#pragma once
#include "Context.h"
#include "Schedule.h"
#include "base/Promise.h"
#include "base/function_traits.h"
#include "base/system/Coroutine.h"

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


class GDefaultSchedule: public GSchedule {
public:
  GDefaultSchedule(GContext* context);
  ~GDefaultSchedule();

  Future<int>&& schedule();

  template<typename F, class Ret = typename gql::function_traits<F>::return_type, typename ...Args>
  decltype(auto) async(F&& f, Args ...args) {
    gql::Core* core = new gql::Core;
    core->_schedule = this;
    core->_state = gql::Start;
    core->_listeners.push_back(new gql::Executor(std::forward<F>(f), args...));
    _cores.push_back(core);
    return Promise<Ret>(core);
  }

  void run();

  friend void __coroutine_entry(transfer_t t);
  friend class GCoroutine;
private:

  virtual bool init(GWorker* c);

  void join();

  size_t size() const;

  /**
   * Build plan graph from start node to end node
   */
  int buildPlanGraph(GPlaneNode* root, GPlaneNode* tail);

private:
  uint8_t _current;
  fcontext_t _main;
  stack_allocator _alloc;
  uint8_t _maxID;
  std::map<uint8_t, GCoroutine*> _coroutines;

  std::list<gql::Core*> _cores;
};