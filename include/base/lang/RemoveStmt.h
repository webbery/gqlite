#pragma once
#include <string>

class GRemoveStmt {
public:
  GRemoveStmt(const std::string& name);

private:
  std::string _name;
};