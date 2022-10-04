#include "operand/algorithms/DecisionTree.h"

int GDecisionTree::addNode(const std::string& prop, const std::string& op /*= ""*/)
{
  if (op.empty()) {
    tree_t::node_collection collection;
    _tree.nodes()[prop] = collection;
  }
  return 0;
}
