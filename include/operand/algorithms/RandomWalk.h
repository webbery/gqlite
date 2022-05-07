#pragma once
#include <list>
#include "SubGraph.h"
#include <random>

class GRandomWalk {
public:
  GRandomWalk(const GSubGraph& g, double dumping = 0.9);
  
  GVertex* next();

private:
  std::default_random_engine _re;
  std::normal_distribution<> _distribution;
  double _dumping;
  GVertex* _current;
};
