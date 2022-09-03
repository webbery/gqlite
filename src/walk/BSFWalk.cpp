#include "walk/BSFWalk.h"

GBSFWalk::GBSFWalk(const std::string& prop)
  :_prop(prop)
{

}

void GBSFWalk::stand(virtual_graph_t& vg)
{

}

int GBSFWalk::walk(virtual_graph_t& vg, std::function<void(node_t, const node_info&)> f)
{
  return WalkResult::WR_Visited | WalkResult::WR_Preload;
}
