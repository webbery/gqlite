#include "base/gvm/Compiler.h"
#include "base/system/exception/CompileException.h"
#include <limits>

int16_t resolveLocal(const Compiler* compiler, const std::string& name) {
  for (int16_t i = compiler->_count - 1; i >= 0; --i) {
    const LocalVariant& lv = compiler->_variant[i];
    if (lv._name == name) {
      if (lv._depth == -1) {
        throw GCompileException("use uninitialize variant `%s`.", name.c_str());
      }
      return i;
    }
  }
  return -1;
}


void Compiler::emit(uint8_t byte) const {
  currentChunk().push(byte);
}

void Compiler::namedVariant(const std::string& name, bool isAssign) const{
  uint8_t opGet, opSet;
  short index = resolveLocal(this, name);
  if (index == -1) {
    opSet = (uint8_t)OpCode::OP_SET_GLOBAL;
    opGet = (uint8_t)OpCode::OP_GET_GLOBAL;
  } else {
    opSet = (uint8_t)OpCode::OP_SET_LOCAL;
    opGet = (uint8_t)OpCode::OP_GET_LOCAL;
  }
  if (isAssign) {
    emit(opSet, index);
  } else {
    emit(opGet, index);
  }
}

void Compiler::addLocal(const std::string& name) {
  if (_count == std::numeric_limits<uint8_t>::max()) {
    throw GCompileException("local variant num up to max.");
  }
  auto& var = _variant[_count++];
  var._name = name;
  var._depth = _scopeDepth;
}

void Compiler::declLocal(const std::string& name) {
  // check variant is exist in current scope 
  for (int16_t i = _count - 1; i >= 0; --i) {
    LocalVariant& lv = _variant[i];
    if (lv._depth < _scopeDepth) break;

    if (lv._name == name) {
      throw GCompileException("variant `%s` exist.", name.c_str());
    }
  }

  addLocal(name);
}

void Compiler::call(bool assign) {

}

uint8_t Compiler::argumentList() {
  return 0;
}


