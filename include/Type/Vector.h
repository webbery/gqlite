#pragma once
#include <stdint.h>
namespace gql {
  template <typename Tp>
  class GVector {};

  using vector_double = GVector<double>;
  using vector_uint8 = GVector<uint8_t>;
}
