#pragma once
#if __cplusplus >= 201103L
#include <initializer_list>
#endif
#include <map>
#include <set>
#include "parlay/sequence.h"
#include "json.hpp"
#include "Graph/Node.h"
#include "Graph/Edge.h"

#define MAX_LAYER_SIZE  4

class GNode; 
class GMap {
public:
  enum class node_t : uint64_t {};
  using node_attr_t = uint8_t;
  using node_literal_t = uint32_t;

  using nodes_t = std::pair<node_t, node_t>;
  using node_attrs_t = parlay::sequence<node_attr_t>;

  enum class edge_t : uint64_t {};
  using edge_attr_t = uint8_t;

  using edges_t = parlay::sequence<edge_t>;
  using edge_attrs_t = parlay::sequence<edge_attr_t>;

  // <edges, attribute nodes, json info>
  // keep attribute for match topo structure
  using node_collection = std::tuple<std::array<edges_t, MAX_LAYER_SIZE>, node_attrs_t, nlohmann::json, NodeStatus>;
  using edge_collection = std::tuple<nodes_t, edge_attrs_t, nlohmann::json, EdgeStatus>;
  using pam_node = std::map<node_t, node_collection>;
  using pam_edge = std::map<edge_t, edge_collection>;

  enum {
    no_node = UINT64_MAX,
  };
  ~GMap() {
#if defined(GQLITE_ENABLE_PRINT)
    printf("~GMap()\n");
#endif
  }

  size_t size() const { return _nodes.size() + _edges.size(); }
  size_t node_size() const { return _nodes.size(); }
  size_t edge_size() const { return _edges.size(); }

  size_t visited_size() const { return _visitedNodes.size(); }

  bool visit(node_t node, node_collection& collections) {
    if (_nodes.count(node)) {
      std::get<3>(_nodes[node]).visit = true;
      collections = _nodes[node];
      return true;
    }
    return false;
  }

  bool is_visited(node_t node) {
    if (_nodes.count(node)) {
      return std::get<3>(_nodes[node]).visit;
    }
    return false;
  }

  void clean() {
    parlay::parallel_for(0, _visitedNodes.size(), [&](size_t idx) {
      node_t nodeID = _visitedNodes[idx];
    });
  }

  void clear() {
    _visitedNodes.clear();
  }

  pam_node::const_iterator node_begin() const {
    return _nodes.begin();
  }

  pam_node::const_iterator node_end()const {
    return _nodes.end();
  }

  /**
   * return true if id's neighbors exist.
   */
  bool neighbors(node_t id, std::set<node_t>& n, uint8_t layer = 0) {
    if (_nodes.count(id) != 0) {
      node_collection collection = _nodes[id];
      auto& edges = std::get<0>(collection);

      std::for_each(edges[layer].begin(), edges[layer].end(), [&](edge_t edgeID) {
        auto status = std::get<3>(_edges[edgeID]);
        if (status.del == false) {
          node_t to = std::get<0>(_edges[edgeID]).second;
          if (status.direction == 0 && to == id) {
            to = std::get<0>(_edges[edgeID]).first;
          }
          n.insert(to);
        }
        });
    }
    if (n.size() == 0) {
      std::for_each(_nodes.begin(), _nodes.end(), [&n, id](const std::pair<node_t, node_collection>& item) {
        if (item.first == id) return;
        n.insert(item.first);
        });
      return false;
    }
    return true;
  }

  pam_node& nodes() { return _nodes; }
  const pam_node& nodes() const { return _nodes; }
  pam_edge& edges() { return _edges; }
  const pam_edge& edges() const { return _edges; }

  
private:
  parlay::sequence<node_t> _visitedNodes;
  pam_node _nodes;
  pam_edge _edges;
};

using node_t = GMap::node_t;
using edge_t = GMap::edge_t;

inline edge_t operator++(edge_t& e) {
  edge_t t = e;
  e = static_cast<edge_t>(static_cast<uint64_t>(e) + 1);
  return t;
}

inline edge_t& operator++(edge_t& e, int) {
  e = static_cast<edge_t>(static_cast<uint64_t>(e) + 1);
  return e;
}