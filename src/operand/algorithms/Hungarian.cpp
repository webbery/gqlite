#include "operand/algorithms/Hungarian.h"
#include "gqlite.h"
#include <fmt/printf.h>
#include <iostream>

int HungorianAlgorithm::solve(Eigen::MatrixXd& m) {
  const auto minMat = m.rowwise().minCoeff();
  std::cout<< minMat<<std::endl;
  return ECode_Success;
}