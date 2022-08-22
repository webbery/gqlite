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
  using edge_t = GMap::edge_t;
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
  
  /**
   * @brief 
   * 
   * @param id 
   * @param attr 
   * @param value 
   */
  void addNode(node_t id, const std::vector<node_attr_t>& attr, const nlohmann::json& value);

  /**
   * @brief add an edge. It only discribe one direction. If it is a bidirection, another edge must be added.
   * 
   * @param id edge id
   * @param from out node
   * @param to in node
   * @param attr edge attributes index
   * @param value edge attributes data
   */
  void addEdge(edge_t id, node_t from, node_t to,
    const std::vector<node_attr_t>& attr = std::vector<node_attr_t>(), const nlohmann::json& value = nlohmann::json());
  void release();

  /**
   * @brief visit virtual network
   * 
   * @tparam Visitor 
   * @tparam DataLoader 
   * @param selector select which node to be visit, such BSF/random walk
   * @param visitor how to visit node's element
   * @param loader if node is not exist, try to load a batch nodes that will be visited.
   */
  template<typename Visitor, typename DataLoader>
  void visit(VisitSelector selector, Visitor visitor, DataLoader loader) {
    GWalkFactory* factory = new GWalkFactory();
    std::shared_ptr<IWalkStrategy> strategy = factory->createStrategy(selector);

    // wait for start
    _event.on((int)VNMessage::FirstNodeLoaded, [&_vg = this->_vg, visitor, strategy, loader](const std::any&) {
      WalkResult result = strategy->walk(_vg, visitor);
      if (result & WalkResult::WR_Preload) {
        loader.load();
      }
    });
    _event.on((int)VNMessage::LastNodeLoaded, [](const std::any&) {
    });
    _event.on((int)VNMessage::WalkInterrupt, [](const std::any&) {
    });
    _event.on((int)VNMessage::WalkStop, [](const std::any&) {
    });
    loader.load();
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