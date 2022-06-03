#pragma once
#include <string>

struct GASTNode;
class GCreateStmt {
public:
  GCreateStmt(const std::string& name, GASTNode* indexes);

  std::string name() const;
  GASTNode* indexes() const;
private:
  std::string _name;
  GASTNode* _indexes;
};