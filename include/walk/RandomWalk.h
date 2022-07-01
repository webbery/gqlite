#pragma once
#include <list>
#include <random>
#include "WalkFactory.h"

class GRandomWalk : public IWalkStrategy {
public:
  GRandomWalk(double dumping = 0.9);

  virtual int walk(virtual_graph_t& vg, std::function<void(GNode*)>);

private:
  // GVertex* next();
  
private:
  std::default_random_engine _re;
  std::normal_distribution<> _distribution;
  double _dumping;
};
