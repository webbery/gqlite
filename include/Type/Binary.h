#pragma once
#include <vector>
#include <string>
#include <stdint.h>

namespace gql
{
  std::vector<uint8_t> base64_decode(const char *base64);
  std::string base64_encode(const std::vector<uint8_t> &bin);

} // namespace gql
