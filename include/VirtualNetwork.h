#pragma once
#include <map>
#include <mutex>
#include <condition_variable>
#include <list>
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
  using node_t = GMap<uint64_t, uint64_t>::node_t;
  using edge_t = GMap<uint64_t, uint64_t>::edge_t;
  using node_attr_t = GMap<uint64_t, uint64_t>::node_attr_t;
  using node_literal_t = GMap<uint64_t, uint64_t>::node_literal_t;

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

  /**
   * @brief 
   * 
   * @param id 
   * @param attr 
   * @param value
   * @param level Hierarchical layer level
   */
  int addNode(node_t id, const std::vector<node_attr_t>& attr = {},
              const nlohmann::json& value = nlohmann::json(), uint8_t level = 0);

  int deleteNode(node_t id, bool mark = true);

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
    const std::vector<node_attr_t>& attr = std::vector<node_attr_t>(), const nlohmann::json& value = nlohmann::json(), int8_t level = 0);

  int deleteEdge(edge_t id, bool mark = true);

  void join();

  void release();

  /**
   * @brief visit virtual network. 
   * Note: If network is not completely build, don't use this interface.
   * 
   * @tparam Visitor 
   * @tparam DataLoader 
   * @param selector select which node to be visit, such BSF/random walk
   * @param visitor how to visit node's element
   * @param loader if node is not exist, try to load a batch nodes that will be visited.
   */
  template<typename Selector, typename Visitor, typename DataLoader>
  void visit(Selector& selector, Visitor visitor, DataLoader loader) {
    // wait for start
    _event.on((int)VNMessage::NodeLoaded, [&event = this->_event,&vg = this->_vg, visitor, loader, &selector](const std::any& ) {
      int result = selector.walk(vg, visitor);
      if (result & WalkResult::WR_Preload) {
        std::any a;
        if (loader.load()) {
          event.emit((int)VNMessage::NodeLoaded,a);
        }
        else {
          event.emit((int)VNMessage::LastNodeLoaded, a);
        }
      }
    });
    _event.on((int)VNMessage::LastNodeLoaded, [](const std::any&) {
#if defined(GQLITE_ENABLE_PRINT)
      printf("finish\n");
#endif
      });
    _event.on((int)VNMessage::WalkInterrupt, [](const std::any&) {
    });
    _event.on((int)VNMessage::WalkStop, [](const std::any& a) {
#if defined(GQLITE_ENABLE_PRINT)
      printf("stop\n");
#endif
      });
    std::any a;
    if (loader.load()) {
      selector.stand(_vg);
      _event.emit((int)VNMessage::NodeLoaded, a);
    }
    else {
      _event.emit((int)VNMessage::LastNodeLoaded, a);
    }
  }

  GMap<uint64_t, uint64_t>::node_collection access(node_t key) {
    GMap<uint64_t, uint64_t>::node_collection collection;
    if (!_vg.visit(key, collection)) {
      // load data

    }
    return collection;
  }

  bool neighbors(node_t node, std::set<node_t>& n, int8_t level = 0);
  bool neighbors(node_t node, std::set<edge_t>& n, int8_t level = 0);

  template<typename T>
  bool isVisited(T key) {
    return _vg.is_visited(key);
  }

  size_t node_size();

  node_const_iterator node_begin() const;
  node_const_iterator node_end() const;

  nlohmann::json node_info(node_t id) const;
  nlohmann::json edge_info(edge_t id) const;

  edge_t getEdgeID(node_t from, node_t to) const;

private:
  size_t clean() { _vg.clean(); return 0;}

private:
  size_t _maxMemory;

  GEventEmitter _event;

  std::vector<std::string> _groups;
#if defined(_PRINT_FORMAT_)
public:
#endif
  virtual_graph_t _vg;
};