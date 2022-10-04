#include "base/math/Entropy.h"
#include "base/system/Platform.h"
#include <algorithm>
#include <map>

namespace gql{
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
#else
#endif

  double entropy(const std::vector<double>& probs)
  {
    double ret = 0;
    for (auto i = 0; i < probs.size(); ++i) {
    }
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
    //__m256d dis = _mm256_set1_pd(0.);
#elif defined(__ARM_ARCH__)
#endif
    for (auto i = 0; i < probs.size(); ++i) {
      if (probs[i] == 0) continue;
      ret += -probs[i] * log2(probs[i]);
    }
    return ret;
  }

  GGain::GGain(const Eigen::MatrixXi& features, const std::vector<bool>& target)
  {
    double i = I(target);
    std::map<double, size_t> m;
    // calc every feature's gain
    for (size_t r = 0; r < features.rows(); ++r) {
      double remd = remainder(features.row(r), target);
      m[i - remd] = r;
    }
    for (auto item : m) {
      _indexes.emplace_back(item.second);
    }
  }

  int GGain::operator[](size_t index)
  {
    return _indexes[index];
  }

  double GGain::I(const std::vector<bool>& target)
  {
    double p = 0;
    std::for_each(target.begin(), target.end(), [&p](bool v) {
      if (v == true) p += 1;
    });
    double pp = p / target.size();
    return gql::entropy({ pp, 1 - pp });
  }

  double GGain::remainder(const Eigen::VectorXi& values, const std::vector<bool>& target)
  {
    // feature value -> value count, value's index
    std::map<int, std::vector<size_t>> featuresInfo;
    for (size_t indx = 0; indx < values.size(); ++indx) {
      auto& v = featuresInfo[values[indx]];
      v.emplace_back(indx);
    }
    double remainder = 0;
    for (auto& item : featuresInfo) {
      auto& vIndexes = item.second;
      double p_n = 0.0;
      for (auto i : vIndexes) {
        if (target[i] == true) p_n += 1;
      }
      double prob_p = p_n / vIndexes.size();
      double prob_n = 1 - prob_p;
      remainder += vIndexes.size() * 1.0 / target.size() * gql::entropy({ prob_p, prob_n });
    }
    return remainder;
  }

}


