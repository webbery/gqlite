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
    LastNodeLoaded = 2,
    WalkInterrupt = 3,    //
    WalkStart = 4,
    WalkStop
  };

  GVirtualNetwork(size_t maxMem);

  void startWalk();
  void stopWalk();
  /**
   * 如果当前节点数量达到上限，按如下规则移除节点：
   * 1. 最近最少使用的节点
   * 然后将节点添加进来
   */
  void addNode(uint32_t id);
  void addEdge(uint32_t id);
  void release();

  template<typename Visitor>
  void visit(VisitSelector selector, Visitor visitor) {
    GWalkFactory* factory = new GWalkFactory();
    auto* strategy = factory->createStrategy(selector);
    // wait for start
    _event.on((int)VNMessage::FirstNodeLoaded, [&](const std::any&) {
      strategy->walk(_vg, visitor);
    });
    _event.on((int)VNMessage::LastNodeLoaded, [&](const std::any&) {
      delete strategy;
      delete factory;
    });
    _event.on((int)VNMessage::WalkInterrupt, [&](const std::any&) {
      delete strategy;
      delete factory;
    });
  }


private:
  size_t clean() { _vg.clean(); return 0;}

private:
  size_t _maxMemory;

  GEventEmitter _event;

  virtual_graph_t _vg;

  std::vector<std::string> _groups;
};