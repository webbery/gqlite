#include "base/gvm/GVM.h"
#include "base/gvm/Chunk.h"
#include "base/gvm/Object.h"
#include "base/gvm/Value.h"
#include "base/system/exception/RuntimeException.h"
#include "gqlite.h"
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <fmt/printf.h>
#include <cassert>
#include <list>
#include "base/system/exception/CompileException.h"
#include "base/gvm/Compiler.h"

//#define DEBUG_TRACE_EXECUTION

namespace {
  void runIntriscFunction(const char* name, const std::list<Value>& args) {
    if (strcmp(name, "console.info") == 0) {
      std::string info;
      for (auto& v: args) {
        info += getValueString(v);
      }
      fmt::print("{}\n", info);
    }
    else {
      throw GRuntimeException("call unknow function `%s`", name);
    }
  }
}

GVM::GVM():_frame(MAX_FRAME), _stackSize(0), _frameSize(0) {}

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

void GVM::push(Value&& value) {
  _stack[_stackSize++] = value;
}

void GVM::push(const Value& value) {
  _stack[_stackSize++] = value;
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

    if (lv._name == name && value.empty()) {
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

int GVM::interpret(FunctionObj* entry) {
  if (!entry) {
    return ECode_Fail;
  }
  _stackSize = 0;
  push(entry);
  CallFrame& frame = _frame[_frameSize++];
  frame._func = entry;
  frame._ip = entry->chunk.ptr();
  frame._slots = _stack;
  try {
    return run();
  } catch (const GRuntimeException& exp) {
    fmt::print("{}\n", exp.what());
    _stackSize = 0;
    return ECode_Runtime_Error;
  }catch (const std::runtime_error& err) {
    fmt::print("runtime error occurd: {}\n", err.what());
    _stackSize = 0;
    return ECode_Runtime_Error;
  } catch (const std::exception& err) {
    fmt::print("gvm error occurd: {}\n", err.what());
    _stackSize = 0;
    return ECode_Fail;
  } catch (...) {
    fmt::print("gvm unknow error\n");
    _stackSize = 0;
    return ECode_Fail;
  }
}

int GVM::run() {
  CallFrame& frame = _frame[_frameSize - 1];
#define READ_BYTE() (*frame._ip++)
#define READ_VALUE() (frame._func->chunk._values[READ_BYTE()])
#define READ_STRING() (READ_VALUE().Get<std::string>())
#define READ_INT() (READ_VALUE().Get<int>())
#define BINARY_OP(op) do{\
    Value& left = _stack[--_stackSize];\
    Value& right = _stack[_stackSize - 1];\
    _stack[_stackSize - 1] = std::move(right op left);\
  } while(false);

  OpCode instruction;
  do {
#ifdef DEBUG_TRACE_EXECUTION
    disassembleInstruction(chunk, _ip - &chunk._code[0]);
#endif
    switch (instruction = (OpCode)READ_BYTE()) {
    // case OpCode::OP_NEGATE:   _values.top() = -_values.top(); break;
    case OpCode::OP_INTRINSIC: {
      const std::string& name = READ_STRING();
      int n = READ_BYTE();
      std::list<Value> args;
      for (int i = 0; i < n; ++i) {
        args.push_back(READ_VALUE());
      }
      runIntriscFunction(name.c_str(), args);
      break;
    }
    case OpCode::OP_ADD:      BINARY_OP(+); break;
    case OpCode::OP_SUBTRACT: BINARY_OP(-); break;
    case OpCode::OP_MULTIPLY: BINARY_OP(*); break;
    case OpCode::OP_DIVIDE:   BINARY_OP(/); break;
    case OpCode::OP_CONSTANT: {
      const Value& constant = READ_VALUE();
      _stack[_stackSize++] = constant;
      break;
    }
    case OpCode::OP_RETURN:
       if (_stackSize) --_stackSize;
      return ECode_Success;
    case OpCode::OP_DEF_GLOBAL: {
      const Value& name = READ_VALUE();
      _global[name.Get<std::string>()] = _stack[_stackSize - 1];
      break;
    }
    case OpCode::OP_SET_GLOBAL: {
      const std::string& name = READ_STRING();
      auto itr = _global.find(name);
      if (itr == _global.end()) {
        throw GRuntimeException("undefine variable `%s`", name.c_str());
      }
      itr->second = _stack[_stackSize - 1];
      break;
    }
    case OpCode::OP_GET_GLOBAL: {
      const std::string& name= READ_STRING();
      auto itr = _global.find(name);
      if (itr == _global.end()) {
        throw GRuntimeException("undefine variable `%s`", name.c_str());
      }
      push(itr->second);
      break;
    }
    case OpCode::OP_SET_LOCAL: {
      READ_VALUE() = _stack[_stackSize - 1];
      break;
    }
    case OpCode::OP_GET_LOCAL: {
      push(std::move(READ_VALUE()));
      break;
    }
    case OpCode::OP_POP: {
      --_stackSize;
      break;
    }
    default:
      return ECode_Fail;
    }
  } while(true);
#undef BINARY_OP
#undef READ_VALUE
#undef READ_BYTE
#undef READ_STRING
#undef READ_INT
  return ECode_Success;
}
