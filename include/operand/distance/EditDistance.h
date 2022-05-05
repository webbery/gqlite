#pragma once
#include <queue>
#include "SubGraph.h"
#include "operand/algorithms/Hungarian.h"
#include "base/list.h"

class GSubGraph;
enum GraphMetric {
  WeightedBipartite,
  MaxNum
};

namespace {
  struct CostNode: public GNode<CostNode> {
    CostNode(GVertex* replaced, GVertex* vertex, float cost, CostNode* next)
      : _replaced(replaced), _vertex(vertex), _cost(cost){ _next = next;}
    // vertex that will be replaced
    GVertex* _replaced;
    // replace vertex with this one
    GVertex* _vertex;
    float _cost;
  };

  bool operator < (const CostNode& left, const CostNode& right) {
    return left._cost < right._cost;
  }
  
  GSubGraph apply(const GSubGraph& g, const CostNode* nodes) {
    GSubGraph newgraph = g;

    return newgraph;
  }
}

class GraphEditDistance {
public:
  /**
   * K. Riesen, S. Fankhauser, and H. Bunke. Speeding up graph edit distance computation with a bipartite heuristic
   */
  template<GraphMetric Metric>
  double measure(const GSubGraph& from, const GSubGraph& to) {
    // <vertex of `from` will be replaced, vertexes of `to`>
    std::priority_queue<CostNode> open;
    auto first = from.vertex_begin()->second;
    for (auto itr = to.vertex_begin(), end = to.vertex_end(); itr != end; ++itr) {
      // insert substitude operation, first will be replaced by second
      open.push({first, itr->second, 0, nullptr});
    }
    // insert empty operation for deletion
    open.push({first, nullptr, 0, nullptr});
    HungorianAlgorithm alg;
    while (true) {
      CostNode top = open.top();
      double h_p = 0;
      // int result = alg.solve(, h_p);
    }
    return 0;
  }
private:
};

class GEdgeEditDistance {
public:
  template<GraphMetric Metric>
  double measure(const GSubGraph& g1, const GSubGraph& g2) {
    return 0;
  }
};