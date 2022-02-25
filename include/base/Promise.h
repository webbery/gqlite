#pragma once
#include <future>

template <typename Fn, typename Res>
class GPromise {
public:
  GPromise(Fn&& fn): _fn(fn){}

  await() {
      return _future.get();
  }
private:
  std::future<Res> _future;
  Fn _fn;
};