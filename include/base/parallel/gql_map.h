#pragma once
#if __cplusplus >= 201103L
#include <initializer_list>
#endif
#include <map>
#include "parlay/sequence.h"

class GNode; 
class GMap {
public:
  using node_t = uint32_t;
  using node_attr_t = uint8_t;
  using node_literal_t = uint32_t;

  using nodes_t = parlay::sequence<node_t>;
  using node_attrs_t = parlay::sequence<node_attr_t>;
  using node_literals_t = parlay::sequence<node_literal_t>;

  using edge_t = uint32_t;
  using edge_attr_t = uint8_t;
  using edge_literal_t = uint32_t;

  using edges_t = parlay::sequence<edge_t>;
  using edge_attrs_t = parlay::sequence<edge_attr_t>;
  using edge_literals_t = parlay::sequence<edge_literal_t>;

  // <edges, attribute nodes, literal nodes>
  using node_cellection = std::tuple<edges_t, node_attrs_t, node_literals_t>;
  using edge_cellection = std::tuple<nodes_t, edge_attrs_t, edge_literals_t>;
  using pam_node = std::map<node_t, node_cellection>;
  using pam_edge = std::map<edge_t, edge_cellection>;

  size_t size() { return _mNodesInstance.size(); }

  void clean() {
    parlay::parallel_for(0, _visitedNodes.size(), [&](size_t idx) {
      node_t nodeID = _visitedNodes[idx];
      delete _mNodesInstance[nodeID];
      _mNodesInstance.erase(nodeID);
    });
  }

  void clear() {
    for (auto itr = _mNodesInstance.begin(); itr != _mNodesInstance.end(); ++itr) {
      delete itr->second;
    }
    _mNodesInstance.clear();
    _visitedNodes.clear();
  }
  parlay::sequence<node_t> _visitedNodes;
  // limited size of total nodes
  std::map<node_t, GNode*> _mNodesInstance;
  pam_node _nodes;
  pam_edge _edges;

};