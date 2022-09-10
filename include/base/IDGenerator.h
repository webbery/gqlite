#pragma once

template<typename T = uint32_t>
class GIDGenerator {
public:
  GIDGenerator() :_cur() {}

  T generate() {
    if (_reused.size()) {
      T rest = _reused.front();
      _reused.pop_front();
      return rest;
    }
    return _cur++;
  }

  void recycle(T id) {
    _reused.push_back(id);
  }
private:
  T _cur;
  std::list<T> _reused;
};