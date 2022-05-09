#include "operand/analysis/DegreeCentrality.h"

void DegreeCentrality::analysis(const GSubGraph& g) {
  size_t idx = 0;
  for (auto vitr = g.vertex_begin(), vend = g.vertex_end(); vitr != vend; ++vitr) {
      _value.push_back(vitr->second->edge_size());
  }
}

Eigen::MatrixXd operator - (const DegreeCentrality& left, const DegreeCentrality& right) {
  const std::vector<double>& lv = left.value();
  const std::vector<double>& rv = right.value();
  size_t size = lv.size() + rv.size();
  Eigen::MatrixXd m(size, size);
  for (size_t r = 0; r != lv.size(); ++r) {
    for (size_t c = 0; c != rv.size(); ++c) {
      m(r, c) = abs(lv[r] - rv[c]);
    }
  }
  for (size_t r = 0; r != lv.size(); ++r) {
    for (size_t c = r; c != size; ++c) {
      m(r,c) = lv[r];
    }
  }
  return m;
}