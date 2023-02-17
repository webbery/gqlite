#pragma once
#include "Chunk.h"
#include "base/gvm/Object.h"
#include "base/gvm/Value.h"
#include <stack>
#include <map>
#include <vector>

#define MAX_FRAME   64
#define MAX_STACK   (MAX_FRAME*64)
struct Compiler;

struct CallFrame {
  uint8_t* _ip;
  FunctionObj* _func;
  Value* _slots;
};

class GVM {
public:
  GVM();
  ~GVM();

  int interpret(FunctionObj* entry);

  const Value& result() const { return _final; }

  int setGlobalVariant(const std::string& name, const Value& value);

  void declareLocalVariant(Compiler* compiler, const std::string& name, const Value& value);

  bool isGlobalExist(const std::string& name);

  const Value& getGlobalVariant(const std::string& name) const;

private:
  int run();

  void push(Value&& value);
  void push(const Value& value);

  bool call(FunctionObj* fn, int argCnt);

  bool callValue(const Value& value, int argCnt);

  void frameInfo();

  void registNativeFunction(const char* name, NativeFunc func);

  void unregistNativeFunction(const char* name);
  
private:
  Value _stack[MAX_STACK];
  Value* _stackTop;

  std::vector<CallFrame> _frame;
  int _frameSize;

  Value _final;
  /**
  * @brief Here we use map but not hash_map because of memory considered and program may be not so large.
  *        An optimization can be find here: https://abseil.io/about/design/swisstables
  */
  std::map<std::string, Value> _global;
};