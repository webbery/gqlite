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

/**
 * @brief walk result. If a node is visited, 0 bit will be set 1.
 * 
 */
enum WalkResult {
  WR_UnVisit = 0,
  WR_Visited = 0x1,
  WR_Preload = 0x2,
};

class IWalkStrategy {
public:
  virtual ~IWalkStrategy(){
  }
  virtual int walk(virtual_graph_t& vg, std::function<void(GNode*)>) = 0;
};

class GWalkFactory {
public:
  virtual std::shared_ptr<IWalkStrategy> createStrategy(VisitSelector selector);
  
};