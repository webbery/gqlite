#pragma once
#include <string>

struct GASTNode;
class GGroupStmt {
public:
  GGroupStmt(const std::string& name, GASTNode* props = nullptr, GASTNode* indexes = nullptr);
  ~GGroupStmt();

  std::string name() const { return _name; }
  GASTNode* properties() { return _props; }
  GASTNode* indexes() { return _indexes; }
private:
  std::string _name;
  GASTNode* _props;
  GASTNode* _indexes;
};