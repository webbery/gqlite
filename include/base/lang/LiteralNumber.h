#pragma once
#include "Literal.h"

template<typename T>
class GLiteralNumber : public GLiteral {
public:
  GLiteralNumber(T& value)
  : _value(value) {}

  std::string raw() const {
    return std::to_string(_value);
  }

  PropertyKind kind() const {
    return PropertyKind::Number;
  }
  
private:
  T _value;
};
