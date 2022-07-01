#include "walk/WalkFactory.h"
#include "walk/RandomWalk.h"

IWalkStrategy* GWalkFactory::createStrategy(VisitSelector selector) {
  switch (selector)
  {
  case VisitSelector::RandomWalk:
      return new GRandomWalk();
  default:
      return nullptr;
  }
}