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
  using node_collection = std::tuple<edges_t, node_attrs_t, nlohmann::json, NodeStatus>;
  using edge_collection = std::tuple<nodes_t, edge_attrs_t, nlohmann::json, EdgeStatus>;
  using pam_node = std::map<node_t, node_collection>;
  using pam_edge = std::map<edge_t, edge_collection>;

  size_t size() { return _nodes.size() + _edges.size(); }
  size_t node_size() { return _nodes.size(); }
  size_t edge_size() { return _edges.size(); }

  size_t visited_size() { return _visitedNodes.size(); }

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

  pam_node::iterator node_begin() {
    return _nodes.begin();
  }

  pam_node::iterator node_end() {
    return _nodes.end();
  }

  std::set<node_t> neighbors(node_t id) {
    std::set<node_t> n;
    node_collection collection = _nodes[id];
    edges_t& edges = std::get<0>(collection);
    parlay::parallel_for(0, edges.size(), [&](size_t idx) {
      edge_t& edgeID = edges[idx];
      node_t to = std::get<0>(_edges[edgeID]).second;
      n.insert(to);
    });
    return n;
  }

  pam_node& nodes() { return _nodes; }
  pam_edge& edges() { return _edges; }

  
private:
  parlay::sequence<node_t> _visitedNodes;
  pam_node _nodes;
  pam_edge _edges;

};

using node_t = GMap::node_t;
using edge_t = GMap::edge_t;