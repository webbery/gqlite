#pragma once
#include "../MatchPattern.h"

class CTreeStrategy : public IMatchStrategy {
public:
  virtual int match(SubGraph* subgraph, SubGraph* graph);
};
