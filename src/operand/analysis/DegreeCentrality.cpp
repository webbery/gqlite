#include "operand/analysis/DegreeCentrality.h"

Eigen::MatrixXd DegreeCentrality::analysis(const GSubGraph& g) {
  Eigen::MatrixXd m(g.vertex_size(), g.vertex_size());
  size_t idx = 0;
  for (auto vitr = g.vertex_begin(), vend = g.vertex_end(); vitr != vend; ++vitr) {
      m(idx, idx) = vitr->second->edge_size();
  }
  return m;
}
