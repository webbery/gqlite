#pragma once
#include "pam/pam.h"
#if __cplusplus >= 201103L
#include <initializer_list>
#endif

template<typename Key, typename Value, typename _Compare = std::less<Key>>
class GMap {
  struct entry {
    using key_t = Key;
    using val_t = Value;
    static bool comp(key_t a, key_t b) {
      return _Compare()(a, b);
    }
  };
public:
  size_t size() { return _m.size(); }
  // bool insert() { _m.insert()}
private:
  pam_map<entry> _m;
};