#pragma once
#include <string>

struct GListNode;
class GVariableDecl {
public:
  GVariableDecl(const char* name)
    :_name(name) {}
  ~GVariableDecl();

  const std::string& name() const { return _name; }
private:
  std::string _name;
};