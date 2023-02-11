#pragma once
#include "Chunk.h"
#include "base/gvm/Value.h"
#include <stack>
#include <map>

struct Compiler;
class GVM {
public:
  int interpret(Chunk& chunk);

  const Value& result() const { return _final; }

  int setGlobalVariant(const std::string& name, const Value& value);

  void declareLocalVariant(Compiler* compiler, const std::string& name, const Value& value);

private:
  int run(Chunk& chunk);

private:
  uint8_t* _ip;
  std::vector<Value> _stack;
  Value _final;
  /**
  * @brief Here we use map but not hash_map because of memory considered and program may be not so large.
  *        An optimization can be find here: https://abseil.io/about/design/swisstables
  */
  std::map<std::string, Value> _global;
};