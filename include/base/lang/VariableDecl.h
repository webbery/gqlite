#pragma once
#include <string>

struct GListNode;
class GVariableDecl {
public:
  GVariableDecl(const char* name, GListNode* value)
    :_name(name), _value(value) {}
  ~GVariableDecl();

  const std::string& name() const { return _name; }
  GListNode* value() const { return _value; }
private:
  std::string _name;
  GListNode* _value;
};