#pragma once
#include <map>
#include <mutex>
#include <condition_variable>
#include <list>
#include "walk/WalkFactory.h"
#include "base/type.h"
#include "schedule/DefaultSchedule.h"

class GNode;

class GAttributeNode;
class GSchedule;

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

  GVirtualNetwork(GSchedule* schedule, size_t maxMem);
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
  template<typename Selector, typename DataLoader, typename Visitor>
  void visit(Selector& selector, Visitor visitor, DataLoader loader) {
//     // wait for start、
    ((GDefaultSchedule*)_schedule)->addCoroutine([loader, &isFinish = this->_visitFinish](GCoroutine* co) {
      while (loader.load()) {
        co->yield();
      }
      isFinish = true;
    });
    ((GDefaultSchedule*)_schedule)->addCoroutine([&selector, &vg = this->_vg, &isFinish = this->_visitFinish, visitor](GCoroutine* co) {
      selector.stand(vg);
      while (selector.walk(vg, visitor) & WalkResult::WR_Preload) {
        if (isFinish) break;
        co->yield();
      }
    });
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
  bool _visitFinish{false};

  GSchedule* _schedule{nullptr};

  std::vector<std::string> _groups;

  float _degreeDestribution{0.8f};
#if defined(_PRINT_FORMAT_)
public:
#endif
  virtual_graph_t _vg;
};