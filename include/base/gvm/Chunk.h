#pragma once
#include <vector>
#include <stdint.h>
#include "Value.h"

#define WriteChunk(chunk, byte) \
  if(std::numeric_limits<uint8_t>::max() <= (int32_t)byte) {\
    printf("Warning: constant count is greater than gvm const cache size.\n");\
  }\
  chunk._code.push_back((uint8_t)byte)

enum class OpCode {
  OP_SET_GLOBAL,
  OP_SET_LOCAL,
  OP_GET_GLOBAL,
  OP_GET_LOCAL,
  OP_POP,
  /** < declaration */
  OP_DEF_GLOBAL,
  OP_DEF_LOCAL,
  /** < expression */
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  /** < Control */
  OP_IF,
  OP_ELSE,
  OP_CONSTANT,
  OP_RETURN,
};

struct Chunk {
  std::vector<uint8_t> _code;
  ConstPoolValue _values;
};

int addConstant(Chunk& chunk, const Value& value);

#ifdef _DEBUG
void disassembleChunk(const char* name, const Chunk& chunk);
int disassembleInstruction(const Chunk& chunk, int offset);
#endif