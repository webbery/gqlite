#pragma once
#include "WalkFactory.h"

class GBSFWalk : public IWalkStrategy {
public:
  WalkResult walk(virtual_graph_t& vg, std::function<void(GNode*)>);
};