#pragma once
#include "Chunk.h"

struct FunctionObj {
  int arity;
  Chunk chunk;
  std::string name;

  FunctionObj():arity(0){}
};

typedef Value (*NativeFunc)(int argCnt, Value* args);

/**
* Native Object is a function that contains an primitive method of C/C++
*/
struct NativeObj {
  NativeFunc _func;

  NativeObj(NativeFunc f):_func(f) {}
};

struct ClassObj {

};

