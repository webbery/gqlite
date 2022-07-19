#pragma once
#include <string>

struct GASTNode;
class GPlan;
class GCreateStmt {
public:
  GCreateStmt(const std::string& name, GASTNode* groups, GASTNode* indexes = nullptr);
  ~GCreateStmt();

  std::string name() const;

  GASTNode* groups() const;
  GASTNode* indexes() const;
private:
  std::string _name;
  GASTNode* _groups;
  GASTNode* _indexes;
};