#include "base/gvm/Chunk.h"
#include "base/gvm/Value.h"
#include "fmt/color.h"
#include <fmt/printf.h>

#ifdef _DEBUG
static int simpleInstruction(const char* name, int offset) {
  fmt::print("{}\n", name);
  return offset + 1;
}

static int constantInstruction(const char* name, const Chunk& chunk, int offset) {
  uint8_t constant = chunk.at(offset + 1);
  fmt::printf("%-16s %d [", name, constant);
  printValue(chunk._values[constant]);
  fmt::printf("]\n");
  return offset + 2;
}

static int byteInstruction(const char* name, const Chunk& chunk, int offset) {
  uint8_t slot = chunk.at(offset + 1);
  fmt::printf("%-16s %d\n", name, slot);
  return offset + 2;
}

int dynamicInstruction(const char* name, const Chunk& chunk, int offset) {
  uint8_t slot = chunk.at(offset + 1);
  fmt::printf("%-16s %d\n", name, slot);
  uint8_t skip = chunk.at(offset + 2);
  return offset + skip + 3;
}

int disassembleInstruction(const Chunk& chunk, int offset) {
  fmt::printf("%04d ", offset);
  auto instruction = chunk.at(offset);
  switch ((OpCode)instruction) {
  case OpCode::OP_ADD: return simpleInstruction("OP_ADD", offset);
  case OpCode::OP_SUBTRACT: return simpleInstruction("OP_SUBTRACT", offset);
  case OpCode::OP_MULTIPLY: return simpleInstruction("OP_MULTIPLY", offset);
  case OpCode::OP_DIVIDE: return simpleInstruction("OP_DIVIDE", offset);
  case OpCode::OP_NEGATE: return simpleInstruction("OP_NEGATE", offset);
  case OpCode::OP_CONSTANT: return constantInstruction("OP_CONSTANT", chunk, offset);
  case OpCode::OP_DEF_GLOBAL: return constantInstruction("OP_DEF_GLOBAL", chunk, offset);
  case OpCode::OP_DEF_LOCAL: return constantInstruction("OP_DEF_LOCAL", chunk, offset);
  case OpCode::OP_GET_LOCAL: return byteInstruction("OP_GET_LOCAL", chunk, offset);
  case OpCode::OP_SET_LOCAL: return byteInstruction("OP_SET_LOCAL", chunk, offset);
  case OpCode::OP_SET_GLOBAL: return byteInstruction("OP_SET_GLOBAL", chunk, offset);
  case OpCode::OP_GET_GLOBAL: return byteInstruction("OP_GET_GLOBAL", chunk, offset);
  case OpCode::OP_INTRINSIC: return dynamicInstruction("OP_INTRINSIC", chunk, offset);
  case OpCode::OP_POP: return simpleInstruction("OP_POP", offset);
  case OpCode::OP_RETURN: return simpleInstruction("OP_RETURN", offset);
  default:
    fmt::print("Unknow OpCode {}\n", (uint8_t)instruction);
    return offset + 1;
  }
}

void disassembleChunk(const char* name, const Chunk& chunk) {
  fmt::print("== {} ==\n", name);
  for (int offset = 0, size = chunk.size(); offset < size;) {
    offset = disassembleInstruction(chunk, offset);
  }
}
#endif

int addConstant(Chunk& chunk, const Value& value) {
  chunk._values.push_back(value);
  return chunk._values.size() - 1;
}