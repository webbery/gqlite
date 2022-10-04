#pragma once
#include <cmath>
#include <vector>
#include <Eigen/Core>

namespace gql {
  /**
   * @brief Construct a new GEntropy object
   *
   * @param probs an array of event's probability
   */
  double entropy(const std::vector<double>& probs);

  class GGain {
  public:
    /**
     * @brief Construct a new GGain object
     * 
     * @param features MxN feature matrix, row is feature count, col is feature value
     * @param target 
     */
    GGain(const Eigen::MatrixXi& features, const std::vector<bool>& target);

    /**
     * get gain index. Sort from max to min.
     * For example, index 0 means max gain
     */
    int operator[](size_t index);

    size_t size() { return _indexes.size(); }
  private:
    double I(const std::vector<bool>& target);
    double remainder(const Eigen::VectorXi& values, const std::vector<bool>& target);
  private:
    // sort with gain
    std::vector<int> _indexes;
  };
}
