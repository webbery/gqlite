#pragma once
#include "Schedule.h"
#include "base/system/Coroutine.h"
#include "base/function_traits.h"
#include <atomic>
#include <list>
#include <tuple>

namespace gql{
  ///   Following folly.
  ///   The FSM to manage the primary producer-to-consumer info-flow has these
  ///   allowed (atomic) transitions:
  ///
  ///   +----------------------------------------------------------------+
  ///   |                       ---> OnlyResult -----                    |
  ///   |                     /                       \                  |
  ///   |                  (setResult())             (setCallback())     |
  ///   |                   /                           \                |
  ///   |   Start --------->                              ------> Done   |
  ///   |     \             \                           /                |
  ///   |      \           (setCallback())           (setResult())       |
  ///   |       \             \                       /                  |
  ///   |        \              ---> OnlyCallback ---                    |
  ///   |         \           or OnlyCallbackAllowInline                 |
  ///   |          \                                  \                  |
  ///   |      (setProxy())                          (setProxy())        |
  ///   |            \                                  \                |
  ///   |             \                                   ------> Empty  |
  ///   |              \                                /                |
  ///   |               \                            (setCallback())     |
  ///   |                \                            /                  |
  ///   |                  --------> Proxy ----------                    |
  ///   +----------------------------------------------------------------+
  enum CoreState {
    Start = 1 << 0,
    OnlyResult = 1 << 1,
    OnlyCallback = 1 << 2,
    OnlyCallbackInline = 1 << 3,
    Proxy = 1 << 4,
    Done = 1 << 5,
    Empty = 1 << 6,
  };

  class Executor {
    template<size_t ...Idx>
    struct tuple_index {};

    template<size_t N, size_t ...Idx>
    struct tuple_bind : tuple_bind<N - 1, N - 1, Idx...> {};

    template<size_t ...Idx>
    struct tuple_bind<0, Idx...> {
      typedef tuple_index<Idx...> type;
    };

    struct ArgumentsBase {
      virtual ~ArgumentsBase(){}
      virtual void envoke() = 0;
    };

    template<typename F, typename ...Args>
    struct Arguments : public ArgumentsBase {
      std::tuple<Args...> _args;
      F _func;

      static_assert((sizeof ...(Args)) == gql::function_traits<F>::arity,
        "argument type is not equal with lambda function's params");

      Arguments(F&& f, Args ...args):_func(f), _args(args...) {}
      virtual void envoke() {
        exec(typename tuple_bind<sizeof ...(Args)>::type());
      }

      template<size_t ...Idx>
      void exec(tuple_index<Idx...>&&) {
        _func(std::get<Idx>(_args)...);
      }
    };

  public:
    template<typename F, typename ...Args>
    Executor(F&& f, Args... args) {
      _base = new Arguments<F, Args...>(std::forward<F>(f), args...);
    }

    ~Executor() {
      delete _base;
    }

    void operator()() {
      _base->envoke();
    }

  private:
    ArgumentsBase* _base{};
  };

  struct Core {
    std::atomic< gql::CoreState > _state;
    // current coroutine worker
    GDefaultSchedule* _schedule;
    std::list<Executor*> _listeners;
  };
}

template <class T> class Future;
template <class T> class Promise;

template <typename T>
class Promise {
public:
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise& ) = delete;

  Promise(gql::Core* core): _core(core) {
  }
  ~Promise() {
  }

  Promise(Promise<T>&& other) noexcept {
    set(other);
    other.clean();
  }

  Promise& operator=(Promise<T>&& other) noexcept {
    set(other);
    other.clean();
    return *this;
  }
  
  Future<T>&& getFuture() {
    return std::move(Future<T>(this->_core));
  }

  template <typename F> void setInterruptHandler(F&& fn);

  template<typename F>
  void setCallback(F&& fn) {
    constexpr auto allowed = gql::CoreState::OnlyResult | gql::CoreState::Proxy | gql::CoreState::Start;
    // while (!(_core->_state & allowed)) {
    //   _core->_coroutine->yield();
    // }
    // switch (_core->_state) {
    //   case gql::Start:       _core->_state = gql::CoreState::OnlyCallback; break;
    //   case gql::OnlyResult:  _core->_state = gql::CoreState::Done; break;
    //   case gql::Proxy:       _core->_state = gql::CoreState::Empty; break;
    //   default: throw ;
    // }
    // _schedule->addCoroutine(fn);
  }

  bool setValue(T&& value) {
    constexpr auto allowed = gql::CoreState::OnlyResult | gql::CoreState::OnlyCallback | gql::CoreState::OnlyCallbackInline;
    /*while (!(_core->_state & allowed)) {
      _core->_coroutine->yield();
    }*/
    _core->_state = gql::CoreState::Done;
    return true;
  }

  bool valid() const noexcept { return _core != nullptr; }

private:
  void clean() {
    this->_core = nullptr;
  }

  void set(const Promise& other) {
    this->_core = other._core;
  }

private:
  gql::Core* _core{nullptr};
  // schedule for adding next callback
};