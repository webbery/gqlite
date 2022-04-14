#pragma once

class GSubGraph;
class GraphEditDistance {
public:
  enum Metric {
    WeightedBipartite,
    MaxNum
  };
  double measure(const GSubGraph& g1, const GSubGraph& g2, Metric metric = WeightedBipartite);
private:
};