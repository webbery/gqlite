#pragma once
#include "WalkFactory.h"
#include "walk/AStarWalk.h"

class GBFSHeuristic: public IAStarHeuristic {
public:
  GBFSHeuristic(node_t target): IAStarHeuristic(target), _order(0) {}

  double operator()(const node_info& cur, const node_info& node) {
    return _order += 0.00005;
  }

private:
  double _order;
};

class GBFSSelector : public IAStarWalkSelector<GBFSHeuristic> {
public:
  GBFSSelector(GBFSHeuristic& h) : IAStarWalkSelector(h) {
  }

  void start(node_t from) {
    _pos = from;
  }
};