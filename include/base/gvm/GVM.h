#pragma once
#include "Chunk.h"
#include "base/gvm/Value.h"
#include <stack>
#include <map>

class GVM {
public:
  int interpret(const Chunk& chunk);

  const Value& result() const { return _final; }

  int setGlobalVariant(const std::string& name, const Value& value);
private:
  int run(const Chunk& chunk);

private:
  uint8_t* _ip;
  std::stack<Value> _values;
  Value _final;
  /**
  * @brief Here we use map but not hash_map because of memory considered and program may be not so large.
  *        An optimization can be find here: https://abseil.io/about/design/swisstables
  */
  std::map<std::string, Value> _global;
};