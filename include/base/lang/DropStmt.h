#pragma once
#include <string>

class GDropStmt {
public:
  GDropStmt(const std::string& name);

  std::string name() const { return _name; }
private:
  std::string _name;
};