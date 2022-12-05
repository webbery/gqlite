#pragma once
#include <string>

struct GASTNode;
class GUpsetStmt {
public:
  GUpsetStmt(const std::string& vertexGroup, GASTNode* ast);
  GUpsetStmt(const std::string& vertexGroup, GASTNode* ast, GASTNode* conditions);
  GUpsetStmt(GASTNode* edgeGroup, GASTNode* ast);
  ~GUpsetStmt();

  std::string name() const { return _name; }
  GASTNode* node() const { return _node; }
  GASTNode* conditions() const { return _conditions; }
private:
  std::string _name;
  GASTNode* _node;
  GASTNode* _conditions;
};