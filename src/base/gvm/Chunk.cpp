#include "base/gvm/Chunk.h"
#include "base/gvm/Value.h"
#include "fmt/color.h"
#include <fmt/printf.h>

static int simpleInstruction(const char* name, int offset) {
  fmt::print("{}\n", name);
  return offset + 1;
}

static int constantInstruction(const char* name, const Chunk& chunk, int offset) {
  uint8_t constant = chunk._code[offset + 1];
  fmt::printf("%-16s %d [", name, constant);
  printValue(chunk._values[constant]);
  fmt::printf("]\n");
  return offset + 2;
}

int disassembleInstruction(const Chunk& chunk, int offset) {
  fmt::printf("%04d ", offset);
  auto instruction = chunk._code[offset];
  switch ((OpCode)instruction) {
  case OpCode::OP_ADD: return simpleInstruction("OP_ADD", offset);
  case OpCode::OP_SUBTRACT: return simpleInstruction("OP_SUBTRACT", offset);
  case OpCode::OP_MULTIPLY: return simpleInstruction("OP_MULTIPLY", offset);
  case OpCode::OP_DIVIDE: return simpleInstruction("OP_DIVIDE", offset);
  case OpCode::OP_NEGATE: return simpleInstruction("OP_NEGATE", offset);
  case OpCode::OP_CONSTANT: return constantInstruction("OP_CONSTANT", chunk, offset);
  case OpCode::OP_RETURN: return simpleInstruction("OP_RETURN", offset);
  default:
    fmt::print("Unknow OpCode {}\n", (uint8_t)instruction);
    return offset + 1;
  }
}

void disassembleChunk(const char* name, const Chunk& chunk) {
  fmt::print("== {} ==\n", name);
  for (int offset = 0, size = chunk._code.size(); offset < size;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

int addConstant(Chunk& chunk, const Value& value) {
  chunk._values.push_back(value);
  return chunk._values.size() - 1;
}