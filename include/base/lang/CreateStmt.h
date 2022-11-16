#pragma once
#include <string>

struct GASTNode;
class GPlan;
class GCreateStmt {
public:
  GCreateStmt(const std::string& name, GASTNode* groups);
  ~GCreateStmt();

  std::string name() const;

  GASTNode* groups() const;
private:
  std::string _name;
  GASTNode* _groups;
};