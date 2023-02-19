#pragma once
#include <string>

struct GListNode;
class GAssignStmt {
public:
  GAssignStmt(GListNode* name, GListNode* );
  ~GAssignStmt();

  std::string name() const  { return _name; }
  GListNode* decl() const   { return _node; }
  GListNode* value() const  { return _value; }

  bool isDecl() const { return _decl; }
private:
  std::string _name;
  GListNode* _node;
  GListNode* _value;
  bool _decl;
};