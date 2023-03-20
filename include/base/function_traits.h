#pragma once

#include <cstddef>
#include <tuple>

namespace gql {
  template<typename Ret, typename Cls, typename Mult, typename... Params>
  struct _function_traits{
    using is_multable = Mult;
    enum {arity = sizeof...(Params) };
    using return_type = Ret;

    template<size_t i>
    struct arg {
      typedef typename std::tuple_element<i, std::tuple<Params...>>::type type;
    };
  };

  template<class F> struct function_traits;

  template<class Ret, class Cls, class... Args>
  struct function_traits<Ret(Cls::*)(Args...)> : _function_traits<Ret, Cls, std::true_type, Args...>{};

  template<class Ret, class Cls, class... Args>
  struct function_traits<Ret(Cls::*)(Args...)const> : _function_traits<Ret, Cls, std::false_type, Args...> {};

  template<class F>
  struct function_traits : function_traits<decltype(&F::operator())> {
  };
}