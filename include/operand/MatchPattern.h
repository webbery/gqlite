#pragma once
#include <memory>

enum MatchPattern {
  ClosureTree
};

class SubGraph;
class IMatchStrategy {
public:
  virtual int match(SubGraph* subgraph, SubGraph* graph) = 0;
};

class MatchStrategyContext {
public:
  void setStrategy(IMatchStrategy* strategy) { this->_strategy = std::shared_ptr<IMatchStrategy>(strategy); }

  int match(SubGraph* subgraph, SubGraph* graph) {
      return _strategy->match(subgraph, graph);
  }

private:
  std::shared_ptr<IMatchStrategy> _strategy;
};

class IMatchStrategyFactory {
public:
  virtual IMatchStrategy* createMatchStrategy(MatchPattern) = 0;
};

class GMatchStrategyFactory : public IMatchStrategyFactory{
public:
  virtual IMatchStrategy* createMatchStrategy(MatchPattern);
};

int match(SubGraph* subgraph, SubGraph* graph, MatchPattern pattern);
