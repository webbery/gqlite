#pragma once
#include <string>

namespace gql
{
  
class GDatetime {
public:
  std::string compress();
  GDatetime decompress(const std::string& s);
};

} // namespace gql
