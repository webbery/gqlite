#pragma once
#include "Literal.h"
#include <type_traits>

template<typename T>
class GLiteralNumber : public GLiteral {
public:
  GLiteralNumber(T& value)
  : _value(value) {}

  std::string raw() const {
    return std::to_string(_value);
  }

  AttributeKind kind() const {
    return std::is_floating_point<T>::value ? AttributeKind::Number : AttributeKind::Integer;
  }
  
private:
  T _value;
};
