#pragma once
#include <string>

class GDropStmt {
public:
  GDropStmt(const std::string& name);

private:
  std::string _name;
};