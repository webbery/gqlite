#pragma once
#include <Eigen/Core>
#include <list>

class ICostAlgorithm {
public:
  virtual int solve(const Eigen::MatrixXd& m, double& weight) = 0;
};

// use this method: https://hungarianalgorithm.com/examplehungarianalgorithm.php
class HungorianAlgorithm : public ICostAlgorithm {
public:
  int solve(const Eigen::MatrixXd& m, std::list<std::pair<size_t, size_t>>& out);

  virtual int solve(const Eigen::MatrixXd& m, double& weight);
};