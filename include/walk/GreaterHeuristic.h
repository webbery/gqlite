#pragma once
#include "base/parallel/GraphModel.h"

class GGreaterHeuristic {
public:
  double operator() (const GMap::node_collection& left, const GMap::node_collection& right);

  bool is_finish();
};