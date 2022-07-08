#pragma once
#include <string>

struct GASTNode;
class GPlan;
class GCreateStmt {
public:
  GCreateStmt(const std::string& name, GASTNode* indexes = nullptr);

  std::string name() const;
  GASTNode* indexes() const;
private:
  std::string _name;
  GASTNode* _indexes;
};