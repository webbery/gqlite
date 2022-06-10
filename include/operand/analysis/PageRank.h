#pragma once
#include <Eigen/Core>
#include "SubGraph.h"

class GPageRank {
public:
  void analysis(const GSubGraph& g);

private:
  void analysisOfMatrix(const GSubGraph& g);
};

Eigen::MatrixXd operator - (const GPageRank& left, const GPageRank& right);
