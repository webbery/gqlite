#pragma once
#include <map>
#include <mutex>
#include <condition_variable>
#include "Graph/Node.h"
#include "Graph/Edge.h"
#include "walk/WalkFactory.h"
#include "base/parallel/parlay/sequence.h"
#include "base/system/EventEmitter.h"
#include "base/type.h"

class GNode;

class GEntityNode;
class GAttributeNode;
class GVirtualNetwork {
public:
  using node_t = GMap::node_t;
  using node_attr_t = GMap::node_attr_t;
  using node_literal_t = GMap::node_literal_t;

  enum class VNMessage {
    FirstNodeLoaded = 1,
    LastNodeLoaded = 2,
    WalkInterrupt = 3,    //
    WalkStart = 4,
    WalkPause,
    WalkStop
  };

  GVirtualNetwork(size_t maxMem);
  ~GVirtualNetwork();

  void startWalk();
  void stopWalk();
  
  void addNode(node_t id, const std::vector<node_attr_t>& attr, const std::vector<node_literal_t>& value);
  void addEdge(uint32_t id);
  void release();

  template<typename Visitor>
  void visit(VisitSelector selector, Visitor visitor) {
    GWalkFactory* factory = new GWalkFactory();
    std::shared_ptr<IWalkStrategy> strategy = factory->createStrategy(selector);

    // wait for start
    _event.on((int)VNMessage::FirstNodeLoaded, [strategy](const std::any&) {
      // strategy->walk(_vg, visitor);
    });
    _event.on((int)VNMessage::LastNodeLoaded, [](const std::any&) {
    });
    _event.on((int)VNMessage::WalkInterrupt, [](const std::any&) {
    });
    _event.on((int)VNMessage::WalkStop, [](const std::any&) {
    });
    delete factory;
  }

private:
  size_t clean() { _vg.clean(); return 0;}

private:
  size_t _maxMemory;

  GEventEmitter _event;

  virtual_graph_t _vg;

  std::vector<std::string> _groups;
};