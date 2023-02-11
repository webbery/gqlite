#pragma once
#include <string>
#define MAX_UINT8   255

struct LocalVariant {
  std::string _name;    /**< vairant name */
  int8_t _depth;        /**< scope depth of local variant */
  LocalVariant():_depth(-1){}
};

struct Compiler {
  LocalVariant _variant[MAX_UINT8];
  uint8_t _count;
  int8_t _scopeDepth;
};

void initCompiler(Compiler* compiler);

int16_t resolveLocal(Compiler* compiler, const std::string& name);
