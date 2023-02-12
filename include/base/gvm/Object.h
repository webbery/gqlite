#pragma once
#include "Chunk.h"

struct FunctionObj {
  int arity;
  Chunk chunk;
  std::string name;

  FunctionObj():arity(0){}
};

inline void printFunction(const FunctionObj& function) {
  printf("<fn %s>", function.name.c_str());
}

