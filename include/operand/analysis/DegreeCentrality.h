#pragma once
#include <Eigen/Core>
#include "VirtualNetwork.h"

class GDegreeCentrality {
public:
  void analysis(const GVirtualNetwork& g);

  const std::vector<double>& value() const { return _value; }
private:
  std::vector<double> _value;
};

Eigen::MatrixXd operator - (const GDegreeCentrality& left, const GDegreeCentrality& right);