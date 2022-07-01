#pragma once
#include <map>
#include "Graph/Node.h"
#include "walk/WalkFactory.h"

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
  void add(uint32_t id, GNode* node);
  void release();

  template<typename Visitor>
  void visit(VisitSelector selector, Visitor visitor) {
    GWalkFactory* factory = new GWalkFactory();
    auto* strategy = factory->createStrategy(selector);
    strategy->walk(_vg, visitor);
    delete factory;
  }

private:
  size_t clean();

private:
  GNode* _current;
  size_t _maxMemory;
  std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t>> _ids;
  // following two maps will be replaced with bimap
  std::map<uint32_t, GNode*> _id2node;
  std::map<GNode*, uint32_t> _node2id;

  virtual_graph_t _vg;
};