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
#include <typeinfo>
#include <utility>
#include "base/system/exception/CompileException.h"
#include "base/gvm/Compiler.h"
#include "base/gvm/Object.h"

namespace {
  Value nativeClock(int argCnt, Value* args) {
    return double(clock())/CLOCKS_PER_SEC;
  }

  Value nativePrint(int argCnt, Value* args) {
    for (int i = 0; i < argCnt; ++i) {
      printValue(*(args + i));
    }
    return Value();
  }
}

GVM::GVM(GStorageEngine* storage)
  :_frame(MAX_FRAME), _stackTop(&_stack[0]), _frameSize(0)
  ,_storage(storage) {
  registNativeFunction("clock", nativeClock);
  registNativeFunction("console.info", nativePrint);
}

GVM::~GVM() {
  unregistNativeFunction("console.info");
  unregistNativeFunction("clock");
}


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

bool GVM::isGlobalExist(const std::string& name) {
  auto itr = _global.find(name);
  if (itr == _global.end())
    return false;
  return true;
}

const Value& GVM::getGlobalVariant(const std::string& name) const {
  return _global.at(name);
}

void GVM::push(Value&& value) {
  *_stackTop++ = value;
}

void GVM::push(const Value& value) {
  *_stackTop++ = value;
}

bool GVM::call(FunctionObj* fn, int argCnt) {
  if (argCnt != fn->arity) {
    throw GRuntimeException("Expected %d arguments but got %d.", fn->arity, argCnt);
  }

  if (_frameSize == MAX_FRAME) {
    throw GRuntimeException("Stack Overflow.");
  }
  CallFrame& frame = _frame[_frameSize++];
  frame._func = fn;
  frame._ip = fn->chunk.ptr();
  frame._slots = _stackTop - argCnt - 1;
  return true;
}

bool GVM::callValue(const Value& value, int argCnt) {
  return value.visit([this, argCnt](FunctionObj* func) {
    auto ret = call(func, argCnt);
    return ret;
  },
  [this, argCnt](NativeObj* callee) {
    NativeFunc func = callee->_func;
    Value v = func(argCnt, _stackTop - argCnt);
    _stackTop -= argCnt + 1;
    if (!v.empty()) push(v);
    return true;
  });
}

void GVM::registNativeFunction(const char* name, NativeFunc func) {
  _global[name] = new NativeObj(func);
}

void GVM::unregistNativeFunction(const char* name) {
  delete _global[name].Get<NativeObj*>();
}


void GVM::frameInfo() {
  for (int i = _frameSize - 1; i >=0; --i) {
    CallFrame& frame = _frame[i];
    FunctionObj* func = frame._func;
    size_t ins = frame._ip - func->chunk.ptr() - 1;
    printf("[line -] in %s\n", func->name.c_str());
  }
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
  _stackTop = &_stack[0];
  push(entry);
  callValue(entry, 0);
  try {
    return run();
  } catch (const GRuntimeException& exp) {
    fmt::print("{}\n", exp.what());
    _stackTop = &_stack[0];
    return ECode_Runtime_Error;
  }catch (const std::runtime_error& err) {
    fmt::print("runtime error occurd: {}\n", err.what());
    _stackTop = &_stack[0];
    return ECode_Runtime_Error;
  } catch(const std::bad_cast& err) {
    fmt::print("value error occurd: {}\n", err.what());
    _stackTop = &_stack[0];
    return ECode_Fail;
  }catch (const std::exception& err) {
    fmt::print("gvm error occurd: {}\n", err.what());
    _stackTop = &_stack[0];
    return ECode_Fail;
  } catch (...) {
    fmt::print("gvm unknow error\n");
    _stackTop = &_stack[0];
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
    Value& left = *(--_stackTop);\
    Value& right = *(_stackTop - 1);\
    right = std::move(right op left);\
  } while(false);

  OpCode instruction;
  do {
#ifdef _DEBUG
    extern int disassembleInstruction(const Chunk& chunk, int offset);
    //disassembleInstruction(frame._func->chunk, frame._ip - frame._func->chunk.ptr());
#endif
    switch (instruction = (OpCode)READ_BYTE()) {
    // case OpCode::OP_NEGATE:   _values.top() = -_values.top(); break;
    case OpCode::OP_CALL: {
      int argsCnt = READ_BYTE();
      //fmt::print("----------");
      //for (Value* start = &(_stack[0]); start < _stackTop; ++start) {
      //  printValue(*start);
      //  fmt::print("\n");
      //}
      callValue(*(_stackTop - argsCnt - 1), argsCnt);
      break;
    }
    case OpCode::OP_ADD:      BINARY_OP(+); break;
    case OpCode::OP_SUBTRACT: BINARY_OP(-); break;
    case OpCode::OP_MULTIPLY: BINARY_OP(*); break;
    case OpCode::OP_DIVIDE:   BINARY_OP(/); break;
    case OpCode::OP_CONSTANT: {
      const Value& constant = READ_VALUE();
      *_stackTop++ = constant;
      //printf("OP_CONSTANT value: "); printValue(constant);
      break;
    }
    case OpCode::OP_RETURN: {
      //fmt::print("----------");
      //for (Value* start = &(_stack[0]); start < _stackTop; ++start) {
      //  printf("return value: "); printValue(*start);
      //  fmt::print("\n");
      //}
      Value& result = *(--_stackTop);
      --_frameSize;
      if (_frameSize == 0) {
        //printf("return value: "); printValue(result);
        //printValue(*_stackTop); printf("\n");
        //--_stackTop;
        return ECode_Success;
      }
      _stackTop = frame._slots;
      push(result);
      frame = _frame[_frameSize - 1];
      break; 
    }
    case OpCode::OP_DEF_GLOBAL: {
      const Value& name = READ_VALUE();
      _global[name.Get<std::string>()] = *_stackTop;
      break;
    }
    case OpCode::OP_SET_GLOBAL: {
      const std::string& name = READ_STRING();
      auto itr = _global.find(name);
      if (itr == _global.end()) {
        throw GRuntimeException("undefine variable `%s`", name.c_str());
      }
      itr->second = *_stackTop;
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
      frame._slots[READ_BYTE()] = *(_stackTop - 1);
      //printf("set local: ");  printValue(*(_stackTop - 1)); printf("\n");
      break;
    }
    case OpCode::OP_GET_LOCAL: {
      Value v = frame._slots[READ_BYTE()];
      //printf("get local: ");  printValue(v); printf("\n");
      push(v);
      break;
    }
    case OpCode::OP_POP: {
      --_stackTop;
      break;
    }
    case OpCode::OP_JUMP: {
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
