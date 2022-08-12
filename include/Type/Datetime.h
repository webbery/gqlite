#pragma once
#include <string>

namespace gql
{
  
class GDatetime {
public:
  GDatetime(time_t t);
  bool operator == (const GDatetime& other);
  bool operator != (const GDatetime& other);
  bool operator < (const GDatetime& other);
  bool operator > (const GDatetime& other);

private:
  time_t _t;
};

} // namespace gql
