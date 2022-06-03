#pragma once
#include <string>
#include "../type.h"

class GLiteral {
public:
  virtual ~GLiteral() {}

  virtual std::string raw() const = 0;
  virtual PropertyKind kind() const = 0;
};