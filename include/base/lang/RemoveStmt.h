#pragma once
#include <string>

struct GASTNode;
class GRemoveStmt {
public:
  GRemoveStmt(const std::string& name, GASTNode* array);
  ~GRemoveStmt();

  std::string name() const { return _name; }
  GASTNode* node() const { return _array; }
private:
  std::string _name;
  GASTNode* _array;
};