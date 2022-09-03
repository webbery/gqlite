#include "walk/WalkFactory.h"
#include "walk/RandomWalk.h"
#include "walk/BSFWalk.h"
#include "walk/AStarWalk.h"

std::shared_ptr<IWalkStrategy> GWalkFactory::createStrategy(VisitSelector selector, const std::string& prop,
  std::function<double(const IWalkStrategy::node_info& ,const IWalkStrategy::node_info&)> f) {
  switch (selector)
  {
  case VisitSelector::RandomWalk:
    return std::shared_ptr<IWalkStrategy>(new GRandomWalk(prop));
  case VisitSelector::BreadSearchFirst:
    return std::shared_ptr<IWalkStrategy>(new GBSFWalk(prop));
  case VisitSelector::AStarWalk:
  {
    auto walk = new GAStarWalk(prop);
    walk->setHeuristic(f);
    return std::shared_ptr<IWalkStrategy>(walk);
  }
  default:
      return nullptr;
  }
}