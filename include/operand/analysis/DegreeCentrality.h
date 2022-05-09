#pragma once
#include <Eigen/Core>
#include "SubGraph.h"

class DegreeCentrality {
public:
  void analysis(const GSubGraph& g);

  const std::vector<double>& value() const { return _value; }
private:
  std::vector<double> _value;
};

Eigen::MatrixXd operator - (const DegreeCentrality& left, const DegreeCentrality& right);