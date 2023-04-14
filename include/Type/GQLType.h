#pragma once

#include "Binary.h"
#include "Datetime.h"
#include "Hash.h"

#include "base/type.h"
#include "json.hpp"

namespace gql {
  using vector_double = std::vector<double>;
  using vector_uint8 = std::vector<uint8_t>;
}

using gkey_t = Variant<std::string, uint64_t>;
using attribute_t = Variant<std::string, double, gql::GDatetime, gql::vector_double, gql::GBinary, int, long, uint64_t,
  gql::vector_uint8
>;
