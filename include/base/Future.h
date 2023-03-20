#pragma once
#include "base/Promise.h"
#include "base/system/Coroutine.h"
#include "base/function_traits.h"
#include "Schedule.h"
#include "schedule/DefaultSchedule.h"

namespace gql {
  struct Core;

  template<typename F, typename... Args>
  using ResultOf = decltype(std::declval<F>()(std::declval<Args>()...));

  template<typename F, typename... Args>
  struct CallableWith {
    template<typename T, typename = ResultOf<T, Args...>>
    static constexpr std::true_type check(std::nullptr_t) { return std::true_type{}; }

    template<typename> static constexpr std::false_type check(...) { return std::false_type{}; }

    typedef decltype(check<F>(nullptr)) type;

    static constexpr bool value = type::value;
  };

// template<typename T, typename F>
// struct callableResult {
//   typedef typename std::conditional< CallableWith<F>::value,
//     detail::argResult<false, F>,
//     typename std::conditional<
//       CallableWith<F, T&&>::value,
//       detail::argResult<false, F, T&&>,
//       typename std::conditional<
//         CallableWith<F, T&>::value,
//         detail::argResult<false, F, T&>,
//         typename std::conditional<
//           CallableWith<F, Try<T>&&>::value,
//           detail::argResult<true, F, Try<T>&&>,
//           detail::argResult<true, F, Try<T>&>>::type>::type>::type>::type Arg;
//   typedef isFuture<typename Arg::Result> ReturnsFuture;
//   typedef Future<typename ReturnsFuture::Inner> Return;
// };
}

template<typename Param>
class Future {
public:
  Future(gql::Core* c) :_core(c) {}
  Future(gql::Core* c, Param&& param):_core(c) {}
  Future(const Future& ) = delete;
  Future& operator = (const Future& ) = delete;

  template<typename F, class Ret = typename gql::function_traits<F>::return_type, typename ...Args>
  Future<Ret> then(F&& f, Args ...args) {
    _core->_listeners.emplace_back(new gql::Executor(std::forward<F>(f), args...));
    return Future<Ret>(_core);
  }

  template<typename F, class Ret = typename gql::function_traits<F>::return_type>
  Future<Ret> then(F&& f) {
    _core->_listeners.emplace_back(new gql::Executor(std::forward<F>(f), _result));
    return Future<Ret>(_core);
  }

  Param get() {
    GCoroutine* c = nullptr;
    if (_core->_state.load() != gql::Done) {
      c = _core->_schedule->addCoroutine([core = this->_core](GCoroutine* co) {
        for (auto* executor : core->_listeners) {
          (*executor)();
          co->yield();
        }
      });
    }
    return Param{};
  }

  
private:
  gql::Core* _core;
  Param _result;
};

template<> class Future<void> {
public:
  Future(gql::Core* c) :_core(c) {}

  Future(const Future&) = delete;
  Future& operator = (const Future&) = delete;

  template<typename F, class Ret = typename gql::function_traits<F>::return_type, typename ...Args>
  Future<Ret> then(F&& f, Args ...args) {
    _core->_listeners.emplace_back(new gql::Executor(std::forward<F>(f), args...));
    return Future<Ret>(_core);
  }

  template<typename F, class Ret = typename gql::function_traits<F>::return_type>
  Future<Ret> then(F&& f) {
    _core->_listeners.emplace_back(new gql::Executor(std::forward<F>(f)));
    return Future<Ret>(_core);
  }

  template<typename Ret>
  Ret get() {}
private:
  gql::Core* _core;
};