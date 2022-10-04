#pragma once
#include <vector>

class GDecisionNetwork {
  struct UtilityNode;

  struct ChanceNode {
    float _value;
    std::vector<ChanceNode*> _nexts;
    UtilityNode* _utilityNode;
  };
  struct DecisionNode{
    float _value;
    std::vector<ChanceNode*> _stochasticNodes;
    UtilityNode* _utilityNode;
  };
  struct UtilityNode {
    float _value;
  };
public:
};