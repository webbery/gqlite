#pragma once
#include "Literal.h"
#include <string>

class GLiteralDatetime: public GLiteral {
public:
  GLiteralDatetime(time_t t): _t(t) { /*printf("new Datetime: %lld\n", t);*/}

  virtual std::string raw() const {
    return std::to_string(_t);
  }
  virtual AttributeKind kind() const {
    return AttributeKind::Datetime;
  }

private:
  time_t _t;
};