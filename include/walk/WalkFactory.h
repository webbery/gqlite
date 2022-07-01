#pragma once
#include <queue>
#include <functional>
#include "base/parallel/gql_map.h"

class GNode;
class GEntityNode;
class GAttributeNode;
using virtual_graph_t = GMap;
// using virtual_graph_t = GMap<GNode*, std::tuple<GEntityNode*, GAttributeNode*>>;

enum class VisitSelector {
  BreadSearchFirst,
  DeepSearchFirst,
  AStarSearch,
  Dijk,
  RandomWalk
};

class IWalkStrategy {
public:
  virtual int walk(virtual_graph_t& vg, std::function<void(GNode*)>) = 0;
};

class GWalkFactory {
public:
  virtual IWalkStrategy* createStrategy(VisitSelector selector);
  
};