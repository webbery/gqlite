#pragma once
#include <string>
#define MAX_UINT8   255

struct LocalVariant {
  std::string _name;    /**< vairant name */
  uint8_t _depth;       /**< scope depth of local variant */
};

struct Compiler {
  LocalVariant _variant[MAX_UINT8];
  uint8_t _count;
  uint8_t _scopeDepth;
};

inline void initCompiler(Compiler* compiler) {
  compiler->_count = 0;
  compiler->_scopeDepth = 0;
}