#include "operand/MatchPattern.h"

IMatchStrategy* GMatchStrategyFactory::createMatchStrategy(MatchPattern) {
  return nullptr;
}

int match(IGraph* subgraph, IGraph* graph, MatchPattern pattern) {
  GMatchStrategyFactory* factory = new GMatchStrategyFactory();
  IMatchStrategy* strategy = factory->createMatchStrategy(pattern);
  int ret = strategy->match(subgraph, graph);
  delete strategy;
  delete factory;
  return ret;
}
