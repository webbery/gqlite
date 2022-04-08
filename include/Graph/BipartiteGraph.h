#pragma once
#include <Eigen/Core>
#include <vector>
#include "SubGraph.h"

class GVertex;
class GBipartiteGraph: public GSubGraph {
public:
  GBipartiteGraph(){}
  GBipartiteGraph(const GSubGraph& g);

  void setLeft(const std::vector<GVertex*>& left);
  void setRight(const std::vector<GVertex*>& right);
  void resize();
  void setWeight(size_t left, size_t right, double weight);

  Eigen::MatrixXd toMatrix(const char* weight);
private:
  std::vector<GVertex*> _left;  // row
  std::vector<GVertex*> _right; // col
};

template <> GBipartiteGraph graph_cast<GBipartiteGraph>(const GSubGraph& g);
