#pragma once
#include "Literal.h"
#include <string>

class GLiteralString : public GLiteral {
public:
  GLiteralString(const char* str, size_t len);
  virtual ~GLiteralString();

  virtual std::string raw() const;
  
  AttributeKind kind() const {
    return AttributeKind::String;
  }
private:
  std::string _value;
};
