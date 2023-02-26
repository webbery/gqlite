#pragma once
#include "WalkFactory.h"
#include "walk/AStarWalk.h"

class GBFSHeuristic: public IAStarHeuristic {
public:
  GBFSHeuristic(node_t target): IAStarHeuristic(target), _order(0) {}

  double operator()(const node_info& cur, const node_info& node) {
    return _order += 1;
  }

  double h(node_t id) {
    return 0;
  }

private:
  uint16_t _order;
};

template<typename Graph>
class GBFSSelector : public IAStarWalkSelector<Graph, GBFSHeuristic> {
public:
  GBFSSelector(GBFSHeuristic& h) : IAStarWalkSelector<Graph, GBFSHeuristic>(h) {
  }

  void start(typename Graph::node_t from) {
    IAStarWalkSelector<Graph, GBFSHeuristic >::_pos = from;
  }
};