#pragma once
#include <vector>
#include <string>
#include <stdint.h>

namespace gql
{
  std::vector<uint8_t> base64_decode(const char *base64);
  std::string base64_encode(const std::vector<uint8_t> &bin);

  class GBinary {
  public:
    GBinary(const char* base64);

    bool operator == (const GBinary& other) const;
    bool operator != (const GBinary& other)const;
    bool operator < (const GBinary& other)const;
    bool operator > (const GBinary& other)const;
    bool operator >= (const GBinary& other)const;
    bool operator <= (const GBinary& other)const;

    std::vector<uint8_t> raw() const { return _bin; }
  private:
    std::vector<uint8_t> _bin;
  };
} // namespace gql
