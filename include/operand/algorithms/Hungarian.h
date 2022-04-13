#pragma once
#include <Eigen/Core>
#include <list>

// use this method: https://hungarianalgorithm.com/examplehungarianalgorithm.php
class HungorianAlgorithm {
public:
  int solve(Eigen::MatrixXd& m, std::list<std::pair<size_t, size_t>>& out);
};