#pragma once
#include <string>

struct GListNode;
class GPlan;
class GCreateStmt {
public:
  GCreateStmt(const std::string& name, GListNode* groups);
  ~GCreateStmt();

  std::string name() const;

  GListNode* groups() const;
private:
  std::string _name;
  GListNode* _groups;
};