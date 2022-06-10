#include "operand/analysis/DegreeCentrality.h"

void GDegreeCentrality::analysis(const GSubGraph& g) {
  size_t idx = 0;
  for (auto vitr = g.vertex_begin(), vend = g.vertex_end(); vitr != vend; ++vitr) {
      _value.push_back(vitr->second->edge_size());
  }
}

Eigen::MatrixXd operator - (const GDegreeCentrality& left, const GDegreeCentrality& right) {
  const std::vector<double>& lv = left.value();
  const std::vector<double>& rv = right.value();
  size_t size = lv.size() + rv.size();
  Eigen::MatrixXd m(size, size);
  for (size_t r = 0; r != lv.size(); ++r) {
    for (size_t c = 0; c != rv.size(); ++c) {
      m(r, c) = abs(lv[r] - rv[c]);
    }
  }
  // right top
  for (size_t r = 0; r != lv.size(); ++r) {
    for (size_t c = rv.size(); c != size; ++c) {
      if (r + rv.size()==c) m(r,c) = lv[r];
      else m(r,c) = INFINITY;
    }
  }
  // left bottom
  for (size_t r = lv.size(); r != size; ++r) {
    for (size_t c = 0; c != rv.size(); ++c) {
      if (r - lv.size()==c) m(r,c) = rv[c];
      else m(r,c) = INFINITY;
    }
  }
  // right bottom
  for (size_t r = lv.size(); r != size; ++r) {
    for (size_t c = rv.size(); c != size; ++c) {
      m(r,c) = 0;
    }
  }
  return m;
}