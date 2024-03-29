#pragma once

#include "Binary.h"
#include "Datetime.h"
#include "Hash.h"

#include "base/type.h"
#include "json.hpp"

inline nlohmann::json string2internal_type(const std::string& s) {
  nlohmann::json ret = s;
  return ret;
}

inline AttributeKind getJsonKind(const nlohmann::json& item) {
  return AttributeKind::String;
}

namespace gql {
  using vector_double = std::vector<double>;
  using vector_uint8 = std::vector<uint8_t>;
}
