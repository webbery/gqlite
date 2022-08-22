#include "walk/WalkFactory.h"
#include "walk/RandomWalk.h"
#include "walk/BSFWalk.h"

std::shared_ptr<IWalkStrategy> GWalkFactory::createStrategy(VisitSelector selector) {
  switch (selector)
  {
  case VisitSelector::RandomWalk:
    return std::shared_ptr<IWalkStrategy>(new GRandomWalk());
  case VisitSelector::BreadSearchFirst:
    return std::shared_ptr<IWalkStrategy>(new GBSFWalk());
  default:
      return nullptr;
  }
}