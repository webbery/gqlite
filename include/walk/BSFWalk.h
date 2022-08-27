#pragma once
#include "WalkFactory.h"

class GBSFWalk : public IWalkStrategy {
public:
  int walk(virtual_graph_t& vg, std::function<void(GNode*)>);
};