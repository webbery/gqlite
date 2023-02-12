#pragma once
#include <string>
#include "Object.h"
#define MAX_UINT8   255

struct LocalVariant {
  std::string _name;    /**< vairant name */
  int8_t _depth;        /**< scope depth of local variant */
  LocalVariant():_depth(-1){}
};

struct Compiler {
  FunctionObj* _func;
  LocalVariant _variant[MAX_UINT8];
  uint8_t _count;
  int8_t _scopeDepth;

  Compiler():_count(1), _scopeDepth(0), _func(nullptr) {
    _func = new FunctionObj();
  }
  ~Compiler() { delete _func; }

  Chunk& currentChunk() { return _func->chunk; }
  Chunk& currentChunk() const { return _func->chunk; }
};

int16_t resolveLocal(Compiler* compiler, const std::string& name);
