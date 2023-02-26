#pragma once
#include <stdexcept>
#include <vector>
#include <stdio.h>
#include "base/Variant.h"

struct FunctionObj;
struct NativeObj;
struct ClassObj;
using Value = Variant<double, std::string, bool, int, long, uint64_t, FunctionObj*, NativeObj*, ClassObj*>;
using ConstPoolValue = std::vector<Value>;

void printValue(const Value& value);
std::string getValueString(const Value& value);

Value operator + (const Value& left, const Value& right);

Value operator - (const Value& left, const Value& right);

Value operator * (const Value& left, const Value& right);

Value operator / (const Value& left, const Value& right);