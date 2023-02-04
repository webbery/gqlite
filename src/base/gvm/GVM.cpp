#include "base/gvm/GVM.h"
#include "base/gvm/Chunk.h"
#include "base/gvm/Value.h"
#include "gqlite.h"
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <fmt/printf.h>

//#define DEBUG_TRACE_EXECUTION

int GVM::setGlobalVariant(const std::string& name, const Value& value) {
  auto itr = _global.find(name);
  if (itr != _global.end()) {
    if (itr->second.empty()) {
      itr->second = value;
      return ECode_Success;
    }
    return ECode_Compile_Warn_Var_Exist;
  }
  _global[name] = value;
  return ECode_Success;
}

int GVM::interpret(const Chunk& chunk) {
  _ip = const_cast<uint8_t*>(&chunk._code[0]);
  try {
    run(chunk);
  } catch (const std::runtime_error& err) {
    fmt::printf("runtime error occurd: {}\n", err.what());
    return ECode_Runtime_Error;
  } catch (const std::exception& err) {
    fmt::printf("gvm error occurd: {}\n", err.what());
    return ECode_Fail;
  } catch (...) {
    fmt::printf("gvm unknow error\n");
    return ECode_Fail;
  }
  return ECode_Success;
}

int GVM::run(const Chunk& chunk) {
#define READ_BYTE() (*_ip++)
#define READ_VALUE() (chunk._values[READ_BYTE()])
#define BINARY_OP(op) do{\
    Value& right = _values.top();_values.pop();\
    _values.top() = std::move(right op _values.top());\
  } while(false);

  OpCode instruction;
  do {
#ifdef DEBUG_TRACE_EXECUTION
    disassembleInstruction(chunk, _ip - &chunk._code[0]);
#endif
    switch (instruction = (OpCode)READ_BYTE()) {
    // case OpCode::OP_NEGATE:   _values.top() = -_values.top(); break;
    case OpCode::OP_ADD:      BINARY_OP(+); break;
    case OpCode::OP_SUBTRACT: BINARY_OP(-); break;
    case OpCode::OP_MULTIPLY: BINARY_OP(*); break;
    case OpCode::OP_DIVIDE:   BINARY_OP(/); break;
    case OpCode::OP_CONSTANT: {
      const Value& constant = READ_VALUE();
      _values.push(constant);
      break;
    }
    case OpCode::OP_RETURN:
       if (_values.size()) {
        _final = _values.top();
        _values.pop();
       }
      return ECode_Success;
    case OpCode::OP_DEF_GLOBAL: {
      const Value& name = READ_VALUE();
      _global[name.Get<std::string>()] = _values.top();
      return ECode_Success;
    }
    case OpCode::OP_DEF_LOCAL: {
      return ECode_Success;
    }
    default:
      return ECode_Fail;
    }
  } while(true);
#undef BINARY_OP
#undef READ_VALUE
#undef READ_BYTE
}
