#pragma once
#include <queue>

class GNode;
class GVirtualNetwork {
public:
  GVirtualNetwork(size_t maxMem);
  /**
   * 如果当前节点数量达到上限，按如下规则移除节点：
   * 1. 当前使用次数为0的节点
   * 2. 最近最少使用的节点
   * 然后将节点添加进来
   */
  void addNode(GNode* node);

  void release();

private:
  GNode* _current;
  size_t _maxMemory;
  std::priority_queue<GNode*> _queue;
};