#include "walk/WalkFactory.h"
#include "walk/RandomWalk.h"

std::shared_ptr<IWalkStrategy> GWalkFactory::createStrategy(VisitSelector selector) {
  switch (selector)
  {
  case VisitSelector::RandomWalk:
      return std::shared_ptr<IWalkStrategy>(new GRandomWalk());
  default:
      return nullptr;
  }
}