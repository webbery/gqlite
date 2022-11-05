#pragma once
#include <vector>
#include <stdint.h>
#include "base/system/Platform.h"

namespace gql {

  /**
   * @brief L2 distance
   */
  double distance2(const std::vector<double>& v1, const std::vector<double>& v2);

  double distance2(const std::vector<double>& v);

  uint8_t distance2(const std::vector<uint8_t>& v1, const std::vector<uint8_t>& v2);

  uint8_t distance2(const std::vector<uint8_t>& v);

  bool is_same(const std::vector<double>& v1, const std::vector<double>& v2);

  bool is_same(const std::vector<uint8_t>& v1, const std::vector<uint8_t>& v2);
}