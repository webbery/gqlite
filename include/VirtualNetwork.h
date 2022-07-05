#pragma once
#include <map>
#include <mutex>
#include <condition_variable>
#include "Graph/Node.h"
#include "walk/WalkFactory.h"
#include "base/parallel/parlay/sequence.h"
#include "base/system/EventEmitter.h"
#include "base/type.h"

class GNode;

class GEntityNode;
class GAttributeNode;
class GVirtualNetwork {
public:
  enum class VNMessage {
    FirstNodeLoaded = 1,
  };
  GVirtualNetwork(size_t maxMem);
  /**
   * 如果当前节点数量达到上限，按如下规则移除节点：
   * 1. 最近最少使用的节点
   * 然后将节点添加进来
   */
  void addNode(uint32_t id);
  void release();

  template<typename Visitor>
  void visit(VisitSelector selector, Visitor visitor) {
    GWalkFactory* factory = new GWalkFactory();
    auto* strategy = factory->createStrategy(selector, _visitedNodes);
    // wait for start
    _event.on((int)VNMessage::FirstNodeLoaded, [](const std::any&) {
      // strategy->walk(_vg, visitor);
    });
    delete factory;
  }

  const std::vector<std::string>& attributes() const;
  AttributeKind attributeKind(uint8_t idx) const { return _attributesKind[idx]; }

private:
  size_t clean();

private:
  size_t _maxMemory;

  GEventEmitter _event;

  parlay::sequence<GNode*> _visitedNodes;
  virtual_graph_t _vg;

  std::vector<std::string> _attributes;
  std::vector<AttributeKind> _attributesKind;

  std::vector<std::string> _groups;
};