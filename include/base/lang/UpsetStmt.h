#pragma once
#include <string>

struct GASTNode;
class GUpsetStmt {
public:
  GUpsetStmt(const std::string& graphName, GASTNode* ast);

  std::string name() const { return _name; }
  GASTNode* node() const { return _node; }
private:
  std::string _name;
  GASTNode* _node;
};