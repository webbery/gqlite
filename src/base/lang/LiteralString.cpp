#include "base/lang/LiteralString.h"

GLiteralString::GLiteralString(const char* str, size_t len)
:_value(str, len) {

}

GLiteralString::~GLiteralString() {}

std::string GLiteralString::raw() const {
  return _value;
}