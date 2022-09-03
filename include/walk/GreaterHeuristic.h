#pragma once
#include "base/parallel/gql_map.h"

class GGreaterHeuristic {
public:
  double operator() (const GMap::node_collection& left, const GMap::node_collection& right);

  bool is_finish();
};