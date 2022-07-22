#pragma once
#include <type_traits>
#include <typeindex>
#include <tuple>

namespace gql {
  /**
 * @brief Caculate max size of Args. and put it's type to type
 *
 * @tparam T
 * @tparam Args
 * TODO: MaxSizeType<Args...>::value accour twice, so can be optimized.
 */
  template<typename T, typename... Args>
  struct MaxSizeType : std::integral_constant<int,
    (sizeof(T) > MaxSizeType<Args...>::value) ? sizeof(T) : MaxSizeType<Args...>::value>{
  };

  template<typename T>
  struct MaxSizeType<T>: std::integral_constant<int, sizeof(T)>{};

  template<typename T, typename... CheckList>
  struct IsContain : std::true_type {};

  template<typename T, class Head, typename... Rest>
  struct IsContain<T, Head, Rest...> :
    std::conditional<
    std::is_same<T, Head>::value,
    std::true_type,
    IsContain<T, Rest...>
    >::type {};

  template<typename T>
  struct IsContain<T> : std::false_type {};

  template <typename... Args> struct VariantStorage;

  template<typename T, typename... Args>
  struct VariantStorage<T, Args...> {
    inline static void Release(std::type_index id, void* data) {
      if (id == std::type_index(typeid(T))) {
        static_cast<T*>(data)->~T();
      }
      else {
        VariantStorage<Args...>::Release(id, data);
      }
    }
  };

  template<> struct VariantStorage<> {
    inline static void Release(std::type_index id, void* data) {
    }
  };
}

template<typename... Types>
class Variant {
  using _Storage = gql::VariantStorage<Types...>;

  template<size_t N, typename... Args> struct TypeVisitor;
  template<typename... Args> struct TypeVisitor<0, Args...> { using type = int; };

  template<size_t N, typename Head, typename... Rest>
  struct TypeVisitor<N, Head, Rest...> {
    using type = typename std::conditional <
      N == 0,
      Head,
      typename TypeVisitor<N - 1, Rest...>::type
    >::type;
  };

  template<typename T, typename... Args> struct IndexVisitor;
  template<typename T, typename... Args> struct IndexVisitor { static constexpr size_t value = 0; };
  template<typename T, typename Head, typename... Rest>
  struct IndexVisitor<T, Head, Rest...> {
    static constexpr size_t value = (std::is_same<T, Head>::value ? 0 : IndexVisitor<T, Rest...>::value + 1);
  };

  template<typename T>
  void Set(T&& value) {
    _Storage::Release(_tindex, _data);
    using U = typename std::remove_reference<T>::type;
    U* ptr = new(_data)U(std::forward<T>(value));
    _tindex = std::type_index(typeid(T));
    _indx = IndexVisitor<typename std::remove_cv<U>::type, Types...>::value;
  }
public:
  Variant():_tindex(typeid(void)) {}

  template<typename T,
    class = typename std::enable_if<
      gql::IsContain<typename std::remove_reference<T>::type, Types...>::value
    >::type
  >
  Variant(T&& value):_tindex(typeid(void)){
    Set(value);
  }

  template<typename T>
  Variant operator = (const T& value) {
    Set(value);
    return *this;
  }

  template<typename T>
  T& Get() {
    if (_tindex != std::type_index(typeid(T))) {
      throw std::bad_cast();
    }
    return *(T*)(&_data);
  }

  template<size_t Idx>
  decltype(auto) Get() {
    //static constexpr size_t cnt = sizeof...(Types);
    // static_assert(Idx < cnt);
    using T = typename TypeVisitor<Idx, Types...>::type;
    return *(T*)(&_data);
  }

  size_t index() {
    return _indx;
  }

private:
  char _data[gql::MaxSizeType<Types...>::value];
  std::type_index _tindex;
  size_t _indx;
};

namespace std {
  template <size_t _Idx, class... _Types>
  constexpr decltype(auto) get(
    Variant<_Types...>& _Var) {
    return _Var.template Get<_Idx>();
  }

  template <typename _Type, class... _Types>
  constexpr decltype(auto) get(
    Variant<_Types...>& _Var) {
    return _Var.template Get<_Type>();
  }
}
