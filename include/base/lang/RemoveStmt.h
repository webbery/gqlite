#pragma once
#include <string>

struct GASTNode;
class GRemoveStmt {
public:
  GRemoveStmt(const std::string& name, GASTNode* array);
  ~GRemoveStmt();

private:
  std::string _name;
  GASTNode* _array;
};