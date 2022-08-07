#pragma once
#include <Eigen/Core>
#include "VirtualNetwork.h"

class GPageRank {
public:
  void analysis(const GVirtualNetwork& g);

private:
  void analysisOfMatrix(const GVirtualNetwork& g);
};

Eigen::MatrixXd operator - (const GPageRank& left, const GPageRank& right);
