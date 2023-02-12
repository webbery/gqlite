#include "base/gvm/Compiler.h"
#include "base/system/exception/CompileException.h"

int16_t resolveLocal(Compiler* compiler, const std::string& name) {
  for (int16_t i = compiler->_count - 1; i >= 0; --i) {
    LocalVariant& lv = compiler->_variant[i];
    if (lv._name == name) {
      if (lv._depth == -1) {
        throw GCompileException("use uninitialize variant `%s`.", name.c_str());
      }
      return i;
    }
  }
  return -1;
}
