#pragma once
#include <queue>
#include "base/PACTree.h"

class GNode;

template<typename K, typename T>
class GCacheNode {
public:
private:
  K _key;
  T _value;
  GCacheNode* _next;
  GCacheNode* _prev;
};

template<typename T>
class GLRU {
public:
  GLRU(size_t capacity): _capacity(capacity) {}

private:
  size_t _capacity;
  GCacheNode<uint64_t, GNode*> _node;
};

class GVirtualNetwork {
public:
  GVirtualNetwork(size_t maxMem);
  /**
   * 如果当前节点数量达到上限，按如下规则移除节点：
   * 1. 最近最少使用的节点
   * 然后将节点添加进来
   */
  void add(GNode* node);

  void release();

private:
  void remove(GNode* node);

private:
  GNode* _current;
  GLRU<GNode*> _cache;
  gql::GPACTree<int, GNode> _tree;
};