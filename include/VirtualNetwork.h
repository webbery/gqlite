#pragma once
#include <map>
#include <mutex>
#include <condition_variable>
#include "walk/WalkFactory.h"
#include "base/parallel/parlay/sequence.h"
#include "base/system/EventEmitter.h"
#include "base/type.h"

class GNode;

class GEntityNode;
class GAttributeNode;

class GEmptyHeuristic {
public:
  double operator()(const node_info& cur, const node_info& node) {
    return 0;
  }

  bool success(const std::list<node_t>& ) { return true; }
};

class GVirtualNetwork {
public:
  using node_t = GMap::node_t;
  using edge_t = GMap::edge_t;
  using node_attr_t = GMap::node_attr_t;
  using node_literal_t = GMap::node_literal_t;

  enum class VNMessage {
    NodeLoaded = 1,
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
  int addNode(node_t id, const std::vector<node_attr_t>& attr, const nlohmann::json& value);

  /**
   * @brief add an edge. It only discribe one direction. If it is a bidirection, another edge must be added.
   * 
   * @param id edge id
   * @param from out node
   * @param to in node
   * @param attr edge attributes index
   * @param value edge attributes data
   */
  int addEdge(edge_t id, node_t from, node_t to,
    const std::vector<node_attr_t>& attr = std::vector<node_attr_t>(), const nlohmann::json& value = nlohmann::json());

  void join();

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
  template<typename Selector, typename Visitor, typename DataLoader>
  void visit(Selector& selector, Visitor visitor, DataLoader loader) {
    //GWalkFactory* factory = new GWalkFactory();
    //std::shared_ptr<IWalkStrategy> strategy = factory->createStrategy(selector, prop, heuristic);
    // wait for start
    _event.on((int)VNMessage::NodeLoaded, [this, visitor, loader, &selector](const std::any& ) {
      int result = selector.walk(_vg, visitor);
      if (result & WalkResult::WR_Preload) {
        std::any a;
        if (loader.load()) {
          _event.emit((int)VNMessage::NodeLoaded,a);
        }
        else {
          _event.emit((int)VNMessage::LastNodeLoaded, a);
        }
      }
    });
    _event.on((int)VNMessage::LastNodeLoaded, [](const std::any&) {
      printf("finish\n");
      });
    _event.on((int)VNMessage::WalkInterrupt, [](const std::any&) {
    });
    _event.on((int)VNMessage::WalkStop, [](const std::any& a) {
      printf("stop\n");
      });
    std::any a;
    if (loader.load()) {
      selector.stand(_vg);
      _event.emit((int)VNMessage::NodeLoaded, a);
    }
    else {
      _event.emit((int)VNMessage::LastNodeLoaded, a);
    }
    //delete factory;
  }

  template<typename T, typename Visitor, typename DataLoader>
  void node(T key, Visitor visitor) {
    GMap::node_collection collection;
    if (!_vg.visit(key, collection)) {
      // load data

    }
  }

  std::set<node_t> neighbors(node_t node);

  template<typename T>
  bool isVisited(T key) {
    return _vg.is_visited(key);
  }
private:
  size_t clean() { _vg.clean(); return 0;}

private:
  size_t _maxMemory;

  GEventEmitter _event;

  virtual_graph_t _vg;

  std::vector<std::string> _groups;
};