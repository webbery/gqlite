#include "walk/BSFWalk.h"

WalkResult GBSFWalk::walk(virtual_graph_t& vg, std::function<void(GNode*)> f)
{

  return WalkResult::WR_Visited;
}
