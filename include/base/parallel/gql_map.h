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
  using edge_t = uint32_t;
  using nodes_t = parlay::sequence<node_t>;
  using edges_t = parlay::sequence<edge_t>;
  // <edges, attribute nodes, literal nodes>
  using node_cellection = std::tuple<edges_t,nodes_t,nodes_t>;
  using edge_cellection = std::tuple<nodes_t,edges_t,edges_t>;
  using pam_node = std::map<node_t, node_cellection>;
  using pam_edge = std::map<edge_t, edge_cellection>;

  size_t size() { return _mNodesInstance.size(); }

  // limited size of total nodes
  std::map<node_t, GNode*> _mNodesInstance;
  pam_node _node2edge;
  pam_edge _edge2node;
};