#pragma once
#include <queue>
#include "walk/AStarWalk.h"
#include "operand/algorithms/Hungarian.h"

enum GraphMetric {
  WeightedBipartite,
  MaxNum
};

namespace {
  template <typename T>
  struct GNode {
    GNode* _next;
  };

  struct CostNode: public GNode<CostNode> {
  //  CostNode(GVertex* replaced, GVertex* vertex, float cost, CostNode* next)
  //    : _replaced(replaced), _vertex(vertex), _cost(cost){ _next = next;}
  //  // vertex that will be replaced
  //  GVertex* _replaced;
  //  // replace vertex with this one
  //  GVertex* _vertex;
    float _cost;
  };

  bool operator < (const CostNode& left, const CostNode& right) {
    return left._cost < right._cost;
  }
}

class BipartiteHeuristic {
public:
  BipartiteHeuristic();

  bool success(const std::list<node_t>& l);
};

/**
 * SearchTree is a kind of graph that is used for creating edit state
 */
class SearchTree {
public:
  SearchTree();

private:
};

class GraphEditDistance {
public:
  /**
   * K. Riesen, S. Fankhauser, and H. Bunke. Speeding up graph edit distance computation with a bipartite heuristic
   */
  template<GraphMetric Metric, typename _Graph1, typename _Graph2>
  double measure(const _Graph1& from, const _Graph2& to, float ratio = 0) {
    
    return 0;
  }
private:
};
