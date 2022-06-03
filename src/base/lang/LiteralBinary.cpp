#include "base/lang/LiteralBinary.h"
#include "Type/Binary.h"
#include <vector>

GLiteralBinary::GLiteralBinary(const std::string& raw, const std::string& type)
: _raw(raw)
, _type(type) {
  std::vector<uint8_t> bin = gql::base64_decode(raw.c_str());
  _value.assign(bin.begin(), bin.end());
}