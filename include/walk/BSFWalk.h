#pragma once
#include "WalkFactory.h"

class GBSFWalk : public IWalkStrategy {
public:
  GBSFWalk(const std::string& prop);

  void stand(virtual_graph_t& vg);
  int walk(virtual_graph_t& vg, std::function<void(node_t, const node_info&)>);

private:
  std::string _prop;
};