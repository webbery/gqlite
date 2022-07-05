#include "walk/WalkFactory.h"
#include "walk/RandomWalk.h"

IWalkStrategy* GWalkFactory::createStrategy(VisitSelector selector, parlay::sequence<GNode*>& visitedQueue) {
  switch (selector)
  {
  case VisitSelector::RandomWalk:
      return new GRandomWalk(visitedQueue);
  default:
      return nullptr;
  }
}