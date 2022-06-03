#pragma once
#ifdef _DEBUG
class GDebugMemory {
public:
  static void* allocate(int size);
  static void deallocate(void* ptr);
};
#endif

class GMemory {
public:
  static void* allocate(int size);
  static void deallocate(void* ptr);
};

template<typename T, typename M = GMemory>
class GAllocator {
public:
  using size_type = size_t;
  using value_type = T;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;

  pointer allocate(size_t _Count, const void* _Hint = nullptr) {
    _Count *= sizeof(value_type);
  }
public:
  GAllocator();
  ~GAllocator();

private:
  //void* _cache = nullptr;
  //long* _stack = nullptr;
  //size_t _top_index = 0;
};