#pragma once
#include <list>
#include <random>
#include "WalkFactory.h"
#include <condition_variable>
#include <mutex>

class GRandomWalkSelector {
public:
  GRandomWalkSelector(const std::string& prop, double dumping = 0.9);

  virtual void stand(virtual_graph_t& vg);
  virtual int walk(virtual_graph_t& vg, const std::function<void(node_t, const node_info&)>& f);

private:
  // GVertex* next();
  
private:
  std::string _prop;

  std::default_random_engine _re;
  std::normal_distribution<> _distribution;
  double _dumping;
};


