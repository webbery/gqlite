#pragma once
#include <string>
#include "Object.h"
#define MAX_UINT8   255

struct LocalVariant {
  std::string _name;    /**< vairant name */
  int8_t _depth;        /**< scope depth of local variant */
  LocalVariant():_depth(-1){}
};

struct Compiler {
  FunctionObj* _func;
  LocalVariant _variant[MAX_UINT8];
  uint8_t _count;
  int8_t _scopeDepth;
  Compiler* _parent;

  Compiler(Compiler* parent)
    :_count(1), _scopeDepth(0), _func(nullptr), _parent(parent) {
    _func = new FunctionObj();
  }
  ~Compiler() { delete _func; }

  Chunk& currentChunk() { return _func->chunk; }
  Chunk& currentChunk() const { return _func->chunk; }

  /**
    * @brief For graph script, this function emit byte code to chunk.
    *        Then gvm will envoke the byte code and other plan will retrieve the result.
    */
  void emit(uint8_t byte) const;
    
  template<typename... Types>
  void emit(uint8_t byte, Types... args) const {
    currentChunk().push(byte);
    emit(args...);
  }

  void namedVariant(const std::string& name, bool isAssign) const;
  void addLocal(const std::string& name);
  void declLocal(const std::string& name);

  void call(bool assign);
  uint8_t argumentList();
};

int16_t resolveLocal(Compiler* compiler, const std::string& name);
