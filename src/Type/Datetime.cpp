#include "Type/Datetime.h"

namespace gql
{
  GDatetime::GDatetime(time_t t): _t(t) {}

  bool GDatetime::operator<=(const GDatetime& other) const
  {
    return !operator > (other);
  }

  bool GDatetime::operator>=(const GDatetime& other) const
  {
    return _t >= other._t;
  }

  bool GDatetime::operator>(const GDatetime& other) const
  {
    return !operator < (other);
  }

  bool GDatetime::operator<(const GDatetime& other)const
  {
    return _t < other._t;
  }

  bool GDatetime::operator!=(const GDatetime& other)const
  {
    return !operator == (other);
  }

  bool GDatetime::operator==(const GDatetime& other)const
  {
    return _t == other._t;
  }

} // namespace gql
