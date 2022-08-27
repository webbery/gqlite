#include "walk/BSFWalk.h"

int GBSFWalk::walk(virtual_graph_t& vg, std::function<void(GNode*)> f)
{

  return WalkResult::WR_Visited | WalkResult::WR_Preload;
}
