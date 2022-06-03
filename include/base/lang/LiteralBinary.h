#pragma once
#include "Literal.h"

class GLiteralBinary : public GLiteral {
public:
  GLiteralBinary(const std::string& raw, const std::string& type);

  virtual std::string raw() const {
    return _raw;
  }
  virtual PropertyKind kind() const {
    return PropertyKind::Binary;
  }
private:
  std::string _raw;
  std::string _type;
  std::string _value;
};