#include <catch.hpp>
#include "operand/distance/EditDistance.h"
#include "walk/AStarWalk.h"
#include "Graph/GRAD.h"

TEST_CASE("bipartite heuristic") {
  virtual_graph_t vg;
  GraphPattern gp;
  GraphEditDistance ged;
  ged.measure< GraphMetric::WeightedBipartite >(vg, gp);
}