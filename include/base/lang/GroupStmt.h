#pragma once
#include <string>

struct GASTNode;
class GGroupStmt {
public:
  GGroupStmt(const std::string& name, GASTNode* props = nullptr);
  ~GGroupStmt();

  std::string name() const { return _name; }
  GASTNode* properties() { return _props; }
private:
  std::string _name;
  GASTNode* _props;
};