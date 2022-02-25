#include "Type/Datetime.h"

namespace gql
{
  std::string GDatetime::compress() {
    return "";
  }

  GDatetime decompress(const std::string& s) {
    return GDatetime();
  }
} // namespace gql
