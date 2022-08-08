#pragma once
#include "../MatchPattern.h"

class CTreeStrategy : public IMatchStrategy {
public:
  virtual int match(IGraph* subgraph, IGraph* graph);
};
