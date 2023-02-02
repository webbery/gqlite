#pragma once
#include <string>

struct GListNode;
class GUpsetStmt {
public:
  GUpsetStmt(const std::string& vertexGroup, GListNode* ast);
  GUpsetStmt(const std::string& vertexGroup, GListNode* ast, GListNode* conditions);
  GUpsetStmt(GListNode* edgeGroup, GListNode* ast);
  ~GUpsetStmt();

  std::string name() const { return _name; }
  GListNode* node() const { return _node; }
  GListNode* conditions() const { return _conditions; }
private:
  std::string _name;
  GListNode* _node;
  GListNode* _conditions;
};