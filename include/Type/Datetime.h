#pragma once
#include <string>
#include "base/Variant.h"

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

bool operator<(const double& left, const gql::GDatetime& right);
bool operator<(const int& left, const gql::GDatetime& right);
bool operator>(const double& left, const gql::GDatetime& right);
bool operator>(const int& left, const gql::GDatetime& right);
bool operator<=(const double& left, const gql::GDatetime& right);
bool operator<=(const int& left, const gql::GDatetime& right);
bool operator>=(const double& left, const gql::GDatetime& right);
bool operator>=(const int& left, const gql::GDatetime& right);
bool operator!=(const double& left, const gql::GDatetime& right);
bool operator!=(const int& left, const gql::GDatetime& right);
bool operator==(const double& left, const gql::GDatetime& right);
bool operator==(const int& left, const gql::GDatetime& right);
