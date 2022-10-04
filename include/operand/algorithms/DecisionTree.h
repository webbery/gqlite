#pragma once
#include "base/parallel/GraphModel.h"

template<bool left_open, bool right_open>
struct range{
  double _left;
  double _right;
};

class GDecisionTree {
  using tree_t = GMap<std::string>;
public:
  int load();
  void save();
  void printf();

  int addNode(const std::string& prop, const std::string& op = "");
  int deleteNode();
  int deleteEdge();
  template<typename Range>
  int addEdge(const std::string& from, const std::string& to, const Range& rg) {
    return 0;
  }

private:
  tree_t _tree;
};