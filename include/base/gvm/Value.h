#pragma once
#include <stdexcept>
#include <vector>
#include <stdio.h>
#include "base/Variant.h"

using Value = Variant<double, std::string, bool, int, long, uint64_t>;
using ConstPoolValue = std::vector<Value>;

void printValue(const Value& value);

Value operator + (const Value& left, const Value& right);

Value operator - (const Value& left, const Value& right);

Value operator * (const Value& left, const Value& right);

Value operator / (const Value& left, const Value& right);