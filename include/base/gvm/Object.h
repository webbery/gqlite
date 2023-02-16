#pragma once
#include "Chunk.h"

struct FunctionObj {
  int arity;
  Chunk chunk;
  std::string name;

  FunctionObj():arity(0){}
};

typedef Value (*NativeFunc)(int argCnt, Value* args);

struct NativeObj {
  NativeFunc _func;

  NativeObj(NativeFunc f):_func(f) {}
};

inline void printFunction(const FunctionObj& function) {
  printf("<fn %s>", function.name.c_str());
}

