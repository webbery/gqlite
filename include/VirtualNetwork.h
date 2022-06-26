#pragma once
#include <queue>
#include "base/parallel/gql_map.h"
#include "Graph/Node.h"

class GNode;

class GEntityNode;
class GAttributeNode;
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
  size_t _maxMemory;
  std::priority_queue<uint32_t> _queue;
  GMap<uint32_t, GNode*> _nodes;
  GMap<GNode*, std::tuple<GEntityNode*, GAttributeNode*>> _relations;
};