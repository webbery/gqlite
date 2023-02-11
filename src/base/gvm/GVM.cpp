#include "base/gvm/GVM.h"
#include "base/gvm/Chunk.h"
#include "base/gvm/Value.h"
#include "gqlite.h"
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <fmt/printf.h>
#include <cassert>
#include "base/system/exception/CompileException.h"
#include "base/gvm/Compiler.h"

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

void GVM::declareLocalVariant(Compiler* compiler, const std::string& name, const Value& value) {
  if (compiler->_scopeDepth == 0) {
    // global variant
    if (setGlobalVariant(name, value) == ECode_Compile_Warn_Var_Exist) {
      throw GCompileException("variant `%s` exist.", name.c_str());
    }
    return ;
  }
  if (compiler->_count == std::numeric_limits<uint8_t>::max()) {
    throw GCompileException("local variant num up to max.");
  }
  // check variant is exist in current scope 
  for (int16_t i = compiler->_count - 1; i >= 0; --i) {
    LocalVariant& lv = compiler->_variant[i];
    if (lv._depth < compiler->_scopeDepth) break;

    if (lv._name == name) {
      throw GCompileException("variant `%s` exist.", name.c_str());
    }
  }
  // local variant
  auto& var = compiler->_variant[compiler->_count++];
  var._name = name;
  if (!value.empty()) {
    var._depth = compiler->_scopeDepth;
  }
}

int GVM::interpret(Chunk& chunk) {
  if (_stack.size()) _stack.erase(_stack.begin() + _stack.size() - 1);
  _ip = const_cast<uint8_t*>(&chunk._code[0]);
  try {
    run(chunk);
    //assert(_stack.size() <= 1);
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

int GVM::run(Chunk& chunk) {
#define READ_BYTE() (*_ip++)
#define READ_VALUE() (chunk._values[READ_BYTE()])
#define BINARY_OP(op) do{\
    Value left = _stack.back();\
    _stack.erase(_stack.begin() + _stack.size() - 1);\
    Value& right = _stack.back();\
    left = std::move(right op left);\
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
      _stack.push_back(constant);
      break;
    }
    case OpCode::OP_RETURN:
       if (_stack.size()) {
        _final = _stack.back();
        _stack.erase(_stack.begin() + _stack.size() - 1);
       }
      return ECode_Success;
    case OpCode::OP_DEF_GLOBAL: {
      const Value& name = READ_VALUE();
      _global[name.Get<std::string>()] = _stack.back();
      break;
    }
    case OpCode::OP_DEF_LOCAL: {
      break;
    }
    case OpCode::OP_SET_GLOBAL: {
      uint8_t slot = READ_BYTE();
      break;
    }
    case OpCode::OP_GET_GLOBAL: {
      uint8_t slot = READ_BYTE();
      _stack.push_back(chunk._values[slot]);
      // _values.push(chunk._values[slot]);
      break;
    }
    case OpCode::OP_SET_LOCAL: {
      uint8_t slot = READ_BYTE();
      chunk._values[slot] = _stack.back();
      break;
    }
    case OpCode::OP_GET_LOCAL: {
      uint8_t slot = READ_BYTE();
      _stack.push_back(chunk._values[slot]);
      break;
    }
    case OpCode::OP_POP: {
      _stack.erase(_stack.begin() + _stack.size() - 1);
      break;
    }
    default:
      return ECode_Fail;
    }
  } while(true);
#undef BINARY_OP
#undef READ_VALUE
#undef READ_BYTE
  return ECode_Success;
}
