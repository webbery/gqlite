#pragma once
#include <string>

namespace gql
{
  
class GDatetime {
public:
  GDatetime(time_t t);
  bool operator == (const GDatetime& other) const;
  bool operator != (const GDatetime& other)const;
  bool operator < (const GDatetime& other)const;
  bool operator > (const GDatetime& other)const;
  bool operator >= (const GDatetime& other)const;
  bool operator <= (const GDatetime& other)const;

  time_t value() const { return _t; }
private:
  time_t _t;
};

} // namespace gql
