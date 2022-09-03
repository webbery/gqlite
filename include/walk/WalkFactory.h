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
  AStarWalk,
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
  using node_t = GMap::node_t;
  using node_info = GMap::node_collection;
  
  virtual ~IWalkStrategy(){
  }

  virtual void stand(virtual_graph_t& vg) = 0;
  virtual int walk(virtual_graph_t& vg, std::function<void(node_t, const node_info&)>) = 0;
};

class GWalkFactory {
public:
  virtual std::shared_ptr<IWalkStrategy> createStrategy(VisitSelector selector, const std::string& prop,
    std::function<double(const IWalkStrategy::node_info& ,const IWalkStrategy::node_info&)> f);
  
};