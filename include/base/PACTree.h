#pragma once
#include <tuple>
#include <cassert>
#include <vector>
#include <type_traits>
#include <iostream>

#if (defined __linux) && !(defined (ANDROID))
#define _PURE_CXX _GLIBCXX_PURE
#else
#define _PURE_CXX
#endif

namespace gql {
  namespace detail {
    struct pac_node_base {
      typedef pac_node_base* base_ptr;

      base_ptr _parent;
      base_ptr _left;
      base_ptr _right;
    };

    template<typename Value>
    struct GPACNode : public pac_node_base {
      typedef GPACNode<Value>* link_type;
      typedef typename std::add_pointer<Value>::type value_ptr;
      typename std::add_pointer<Value>::type _begin;
      typename std::add_pointer<Value>::type _end;
    };

    template<typename T>
    bool isFlatten(GPACNode<T>* node) {
      assert(node != nullptr);
      if (node->_end - node->_begin > 1) return true;
      return false;
    }


template<typename T>
GPACNode<T>* fold(GPACNode<T>* tree, GPACNode<T>* ptr) {
  assert(ptr != nullptr);
  if (!tree) return ptr;
  if (tree->_left) {
    ptr = fold(static_cast<GPACNode<T>*>(tree->_left), ptr);
  }
  T* itr = tree->_begin;
  while (itr != tree->_end) {
    ptr->_begin = itr++;
    ++ptr;
  }
  if (tree->_right) {
    ptr = fold(static_cast<GPACNode<T>*>(tree->_right), ptr);
  }
  return ptr;
}

template<typename T>
GPACNode<T>* left_rotate(GPACNode<T>* root) {
  GPACNode<T>* right = root->_right;
  GPACNode<T>* children = right->_left;
  right->_left = root;
  root->_right = children;
  return right;
}

template<typename T>
GPACNode<T>* unfold(GPACNode<T>* begin, GPACNode<T>* end) {
  if (begin == end) {
    return nullptr;
  }
  size_t len = end - begin;
  size_t mid = len / 2;
  GPACNode<T>* left = unfold(begin, begin + mid);
  GPACNode<T>* right = unfold(begin + mid, end);
  GPACNode<T>* node = new GPACNode<T>;
  node->_begin = begin;
  node->_end = begin + 1;
  node->_left = left;
  node->_right = right;
  return node;
}

template<typename T>
GPACNode<T>* unfold(GPACNode<T>* arr) {
  size_t len = arr->_end - arr->_begin;
  if (arr == nullptr || len == 0) return nullptr;
  return unfold(arr->_begin, arr->_end);
}

/**
 * @return a tuple with <left tree, root, right tree>
 */
template<typename T>
std::tuple<GPACNode<T>*, GPACNode<T>*, GPACNode<T>*> expose(GPACNode<T>* tree) {
  if (isFlatten(tree)) {
    GPACNode<T>* root = unfold(tree);
    return {root->_left, root, root->_right};
  }
  return {tree->_left, tree, tree->_right};
}

  inline _PURE_CXX pac_node_base* pac_tree_increment(pac_node_base* p) throw ()
  {
    if (p->_right) {
      pac_node_base* r = p->_right;
      while (r->_left) r = r->_left;
      return r;
    }
    return p->_parent;
  }

  inline _PURE_CXX pac_node_base* pac_tree_decrement(pac_node_base* p) throw ()
  {
    if (p->_left) {
      pac_node_base* r = p->_left;
      while (r->_right) r = r->_right;
      return r;
    }
    return p->_parent;
  }

  template <typename Tp>
  struct pac_iterator {
    // Tp is std::pair
    typedef typename Tp::second_type& reference;
    typedef typename Tp::second_type* pointer;

    typedef typename pac_node_base::base_ptr base_ptr;
    typedef GPACNode<Tp>* link_type;
    typedef typename GPACNode<Tp>::value_ptr node_ptr;

    pac_iterator(): _node(){}
    pac_iterator(link_type node): _node(node), _ptr(node->_begin) {}
    pac_iterator(link_type node, node_ptr ptr): _node(node), _ptr(ptr) {}

    pac_iterator operator++() {
      if (++_ptr == _node->_end) {
        _node = static_cast<link_type>(pac_tree_increment(_node));
      }
      pac_iterator cur(_node, _ptr);
      return cur;
    }
    pac_iterator operator++(int) {
      pac_iterator cur(_node, _ptr);
      if (++_ptr == _node->_end) {
        _node = static_cast<link_type>(pac_tree_increment(_node));
      }
      return cur;
    }

    bool operator != (pac_iterator other) {
      return !this->operator==(other);
    }

    bool operator == (pac_iterator other) {
      return _node == other._node && _ptr == other._ptr;
    }

    reference operator*() const {
      return _ptr->second;
    }
    link_type _node;
    node_ptr _ptr;
  };

  template<typename K, typename V, typename _Compressor, typename Key_Compare>
  class pac_tree {
  public:
    typedef V             value_type;
    typedef GPACNode<V>   node_type;
    typedef GPACNode<V>*  node_ptr;
    typedef pac_iterator<V> iterator;
    typedef const pac_iterator<V> const_iterator;

    pac_tree() = default;
    pac_tree(size_t B): _block(B) {
      _header = new detail::GPACNode<V>;
      _header->_begin = _header->_end = nullptr;
      _header->_left = _header;
      _header->_right = _header;
      _header->_parent = _header;
    }
    ~pac_tree() {
      if (_header) delete _header;
    }
    size_t size() { return _size; }
    node_ptr node(node_ptr left, value_type value, node_ptr right) {
      if (!left) left = _header;
      if (!right) right = _header;
      node_ptr ptr = new node_type;
      ptr->_begin = new value_type(value);
      ptr->_end = ptr->_begin + 1;
      ptr->_left = left;
      ptr->_right = right;
      size_t cnt = size(ptr);
      if (cnt > 4 * _block) {
        return ptr;
      }
      if (cnt >= _block && cnt <= 2 * _block) {
        node_ptr begin = new node_type[cnt];
        fold(ptr, begin);
        return begin;
      }
      return ptr;
    }

    node_ptr join(node_ptr left, value_type value, node_ptr right) {
      if (heavy(left, right)) return left_rotate(right);
      if (heavy(right, left)) return right_rotate(left);
      return node(left, value, right);
    }
    void release() {}

    iterator begin() { return iterator(static_cast<node_ptr>(_header->_parent)); }
    iterator end() { return iterator(_header); }

    node_ptr left_rotate(node_ptr root) {
      return root;
    }

    node_ptr right_rotate(node_ptr root) {
      return root;
    }

    size_t deepth(node_ptr root) {
      if (!root || root == _header->_parent) return 0;
      return std::max(deepth(static_cast<node_ptr>(root->_left)), deepth(static_cast<node_ptr>(root->_right))) + 1;
    }

    bool heavy(node_ptr left, node_ptr right) {
      if (deepth(left) > deepth(right) + 1) return true;
      return false;
    }

    node_ptr _header;
  private:
    size_t size(node_ptr root) {
      if (!root || root == _header) return 0;
      size_t cnt = root->_end - root->_begin;
      return cnt + size(static_cast<typename GPACNode<V>::link_type>(root->_left))+ size(static_cast<typename GPACNode<V>::link_type>(root->_right));
    }
  private:
    size_t _block;
    size_t _size;
  };

  class _Compressor {};
  }

  template<typename K, typename V, typename Compressor=detail::_Compressor>
  class GPACTree {
  public:
    typedef K key_type;
    typedef std::pair<const K, V> value_type;
    typedef detail::pac_tree<key_type, value_type, Compressor, std::less<K>> ptree_type;
    typedef typename ptree_type::iterator iterator;
    typedef typename ptree_type::const_iterator const_iterator;

    GPACTree(): _t(4) {}
    GPACTree(size_t B)
      : _t(B) {
      }

    size_t size() { return _t.size(); }
    
    void set(const K&& key, const V&& value) {
      // typename ptree_type::node_ptr n = _t.node(nullptr, std::make_pair(key, value), nullptr);
      _t.join(_t._header, std::make_pair(key, value), nullptr);
    }

    iterator begin() { return _t.begin(); }
    iterator end() { return _t.end(); }
  private:
    // the actual p-tree structure
    ptree_type _t;
  };
}
