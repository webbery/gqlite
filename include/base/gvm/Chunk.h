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
  OP_CALL,
  /** < Control */
  OP_IF,
  OP_ELSE,
  OP_CONSTANT,
  OP_JUMP,
  OP_RETURN,
  /**< Other */
};

#define MAX_BYTE_CODE   1024
struct Chunk {
  Chunk() :_code(MAX_BYTE_CODE), _codeSize(0) {}

  void push(uint8_t c) {
    _code[_codeSize++] = c;
  }

  uint8_t back() { return _code[_codeSize - 1]; }

  void pop() { --_codeSize; }

  uint8_t at(int index) const { return _code[index]; }

  size_t size() const { return _codeSize; }

  uint8_t* ptr(int idx = 0) { return &_code[idx]; }

  ConstPoolValue _values;
private:
  std::vector<uint8_t> _code;
  short _codeSize;
};

int addConstant(Chunk& chunk, const Value& value);

#ifdef _DEBUG
void disassembleChunk(const char* name, const Chunk& chunk);
int disassembleInstruction(const Chunk& chunk, int offset);
#endif