#pragma once
#include <Eigen/Core>
#include "SubGraph.h"

class DegreeCentrality {
public:
  Eigen::MatrixXd analysis(const GSubGraph& g);
};