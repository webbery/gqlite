#include <catch.hpp>
#include "base/math/Entropy.h"

TEST_CASE("entropy") {
  // 1 - Some, 2 - Full, 0 - None
  Eigen::MatrixXi features(1, 12);
  features <<  1, 2, 1, 2, 2, 1, 0, 1, 2, 2, 0, 2;
  // 0 - No, 1 -Yes
  std::vector<bool> target{ 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1 };
  gql::GGain gain(features, target);
  CHECK(gain[0] == 0);
}