#pragma once
#include <string>

class GRemoveStmt {
public:
  GRemoveStmt(const std::string& name);
  ~GRemoveStmt() {}

private:
  std::string _name;
};