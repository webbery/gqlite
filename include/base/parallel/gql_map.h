#pragma once
#include "pam/pam.h"
#if __cplusplus >= 201103L
#include <initializer_list>
#endif

// template<typename Key, typename Value, typename _Compare = std::less<Key>>
// class GMap {
//   struct entry {
//     using key_t = Key;
//     using val_t = Value;
//     using aug_t = Value;
//     using entry_t = Value;
//     static bool comp(key_t a, key_t b) {
//       return _Compare()(a, b);
//     }
//     static aug_t get_empty() { return aug_t(); }
//     static aug_t from_entry(key_t k, val_t v) { return v; }
//     static aug_t combine(aug_t a, aug_t b) {
//       return a;
//     }
//   };
// public:
//   size_t size() { return _m.size(); }
//   void insert(Value v) { _m.insert(v, [] (const Value& a, const Value& b) {return b;}); }
//   void begin() { return _m.root; }
// private:
//   pam_map<entry> _m;
// };

class GNode; 
class GMap {
  using node_id = uint32_t;
  struct node_entry {
    using key_t = node_id;
    using val_t = GNode*;
    using aug_t = GNode*;
    static bool comp(key_t a, key_t b) { return a < b; }
    static aug_t get_empty() { return 0; }
    static aug_t from_entry(key_t k, val_t v) { return v; }
    static aug_t combine(aug_t a, aug_t b) {
      return (b > a) ? b : a;
    }
    using entry_t = std::pair<key_t,val_t>;
  };
  using node_list = aug_map<node_entry>;

  struct node {
    using key_t = node_id;
    using val_t = node_list;
    static inline bool comp(const key_t& a, const key_t& b) { return a < b;}
  };
  using nodes_t = pam_map<node>;

  nodes_t _m;

  using nodes_elt = std::pair<node_id, typename node_entry::entry_t>;
public:
  using node_t = std::pair<node_id, GNode*>;

  size_t size() { return _m.size(); }

  void merge(const std::vector<node_t>& nodes) {
    auto reduce = [&](parlay::slice<nodes_elt*, nodes_elt*>& R) {
      node_list nl;
      return nl;
    };
    // _m = nodes_t::multi_insert_reduce(nodes_t(), nodes, reduce);
  }

  node_list adjacent(const node_id id) {
    std::optional<node_list> ps = _m.find(id);
    if (ps) return *ps;
    return node_list();
  }
};