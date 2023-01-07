#pragma once
#include <type_traits>
#include <typeindex>
#include <cstring>
#include <tuple>

namespace gql {
  std::string format(const char* fmt, ...);

  class variant_bad_cast : public std::exception {
  public:
    variant_bad_cast(const char* msg): std::exception(
#ifdef WIN32
      msg
  #endif
  ) {}

  #ifdef __linux
    virtual const char*
    what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW {
      return _msg.c_str();
    }

  private:
    std::string _msg;
  #endif
  };

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
    
    inline static void move(std::type_index ot, void* od, void* nd) {
      if (ot == std::type_index(typeid(T))) {
        new(nd) T(std::move(*reinterpret_cast<T*>(od)));
      }
      else {
        VariantStorage<Args...>::move(ot, od, nd);
      }
    }
    
    inline static void copy(std::type_index ot, const void* od, const void* nd) {
      if (ot == std::type_index(typeid(T))) {
        new(const_cast<void*>(nd)) T(*reinterpret_cast<T*>(const_cast<void*>(od)));
      }
      else {
        VariantStorage<Args...>::copy(ot, od, nd);
      }
    }

    inline static bool equal(std::type_index id, void* data1, void* data2) {
      if (id == std::type_index(typeid(T))) {
        return *static_cast<T*>(data1) == *static_cast<T*>(data2);
      }
      else {
        return VariantStorage<Args...>::equal(id, data1, data2);
      }
    }

    inline static bool less_than(std::type_index id, void* data1, void* data2) {
      if (id == std::type_index(typeid(T))) {
        return *static_cast<T*>(data1) < *static_cast<T*>(data2);
      }
      else {
        return VariantStorage<Args...>::less_than(id, data1, data2);
      }
    }
    inline static bool less_than_equal(std::type_index id, void* data1, void* data2) {
      if (id == std::type_index(typeid(T))) {
        return *static_cast<T*>(data1) <= *static_cast<T*>(data2);
      }
      else {
        return VariantStorage<Args...>::less_than_equal(id, data1, data2);
      }
    }
  };

  template<> struct VariantStorage<> {
    inline static void Release(std::type_index id, void* data) {
    }
    inline static void move(std::type_index ot, void* od, void* nd) {}
    inline static void copy(std::type_index ot, const void* od, const void* nd) {}
    inline static bool equal(std::type_index id, void* data1, void* data2) { return false; }
    inline static bool less_than(std::type_index id, void* data1, void* data2) { return false; }
    inline static bool less_than_equal(std::type_index id, void* data1, void* data2) { return false; }
  };

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

  template<class F>
  struct function_traits : function_traits<decltype(&F::operator())> {
  };

  template<class Ret, class Cls, class... Args>
  struct function_traits<Ret(Cls::*)(Args...)> : _function_traits<Ret, Cls, std::true_type, Args...>{};

  template<class Ret, class Cls, class... Args>
  struct function_traits<Ret(Cls::*)(Args...)const> : _function_traits<Ret, Cls, std::false_type, Args...> {};
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
  template<typename T, typename... Args> struct IndexVisitor { static constexpr int value = -1; };
  template<typename T, typename Head, typename... Rest>
  struct IndexVisitor<T, Head, Rest...> {
    static constexpr int value = (std::is_same<T, Head>::value ? 0 : IndexVisitor<T, Rest...>::value + 1);
  };

  template<typename T>
  void Set(T&& value) {
    _Storage::Release(_tindex, &_data);
    using U = typename std::remove_reference<T>::type;
    new(_data)U(std::forward<T>(value));
    _tindex = std::type_index(typeid(T));
    _indx = IndexVisitor<typename std::remove_cv<U>::type, Types...>::value;
  }
public:
  Variant():_tindex(typeid(void)) {}
  ~Variant() {
    _Storage::Release(_tindex, &_data);
  }
  Variant(const Variant& other)
    :_tindex(other._tindex)
    ,_indx(other._indx)
    { _Storage::copy(other._tindex, &other._data, &_data); }
   
  Variant(Variant&& other)
    :_tindex(other._tindex)
    ,_indx(other._indx)
    {
      _Storage::move(other._tindex, &other._data, &_data);
    }

  template<typename T,
    class = typename std::enable_if<
      gql::IsContain<typename std::remove_reference<T>::type, Types...>::value
    >::type
  >
  Variant(T&& value):_tindex(typeid(void)){
    Set(value);
  }

  template<typename T>
  Variant& operator = (const T& value) {
    Set(value);
    return *this;
  }
  
  Variant& operator = (const Variant& other) {
    _tindex = other._tindex;
    _Storage::copy(other._tindex, &other._data, &_data);
    return *this;
  }
  
  Variant& operator = (Variant&& other) {
    _tindex = other._tindex;
    _Storage::move(other._tindex, &other._data, &_data);
    return *this;
  }

  constexpr bool empty() {
    return _indx < 0;
  }

  bool operator == (const Variant& other) const {
    if (other._tindex == this->_tindex && other._indx == this->_indx) {
      return _Storage::equal(_tindex, (void*)&_data, (void*)&other._data);
    }
    static std::string msg;
    msg = gql::format("compare different type, %s and %s\n", _tindex.name(), other._tindex.name());
    throw gql::variant_bad_cast(msg.c_str());
  }

  bool operator < (const Variant& other) const {
    if (other._tindex == this->_tindex && other._indx == this->_indx) {
      return _Storage::less_than(_tindex, &const_cast<Variant&>(*this)._data , &const_cast<Variant&>(other)._data);
    }
    static std::string msg;
    msg = gql::format("compare different type, %s and %s\n", _tindex.name(), other._tindex.name());
    throw gql::variant_bad_cast(msg.c_str());
  }

  bool operator <= (const Variant& other) const {
    if (other._tindex == this->_tindex && other._indx == this->_indx) {
      return _Storage::less_than_equal(_tindex, &const_cast<Variant&>(*this)._data, &const_cast<Variant&>(other)._data);
    }
    static std::string msg;
    msg = gql::format("compare different type, %s and %s\n", _tindex.name(), other._tindex.name());
    throw gql::variant_bad_cast(msg.c_str());
  }

  bool operator > (const Variant& other) const {
    return !operator <= (other);
  }

  bool operator >= (const Variant& other) const {
    return !operator < (other);
  }

  template<typename T>
  T& Get() const {
    //if (_tindex != std::type_index(typeid(T))) {
      //throw std::bad_cast();
    //}
    return *(T*)(&_data);
  }

  template<int Idx>
  decltype(auto) Get() const {
    //static constexpr size_t cnt = sizeof...(Types);
    // static_assert(Idx < cnt);
    using T = typename TypeVisitor<Idx, Types...>::type;
    //static_assert(std::is_same<T, std::string>::value);
    return *(T*)(&_data);
  }

  int index() const {
    return _indx;
  }

  template<typename Func>
  decltype(auto) visit(Func&& f) const {
    using T = typename std::remove_cv<typename gql::function_traits<Func>::template arg<0>::type>::type;
    using Ret = typename gql::function_traits<Func>::return_type;
    if (_tindex == typeid(T)) {
      return f(Get<T>());
    }
    throw std::bad_cast();
  }
  template<typename Func, typename... Rest>
  decltype(auto) visit(Func&& f, Rest&&... rest) const {
    using T = typename std::remove_cv<typename gql::function_traits<Func>::template arg<0>::type>::type;
    if (_tindex == typeid(T)) {
      return visit(std::forward<Func>(f));
    }
    else {
      return visit(std::forward<Rest>(rest)...);
    }
  }

private:
  char _data[gql::MaxSizeType<Types...>::value];
  std::type_index _tindex;
  int _indx;
};

namespace std {
  //template <size_t _Idx, class... _Types>
  //constexpr decltype(auto) get(
  //  Variant<_Types...>& _Var) {
  //  return _Var.template Get<_Idx>();
  //}

  template <typename _Type, class... _Types>
  constexpr decltype(auto) get(
    Variant<_Types...>& _Var) {
    return _Var.template Get<_Type>();
  }
}
