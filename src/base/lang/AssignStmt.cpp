#include "base/lang/AssignStmt.h"
#include "base/lang/AST.h"
#include "base/lang/ASTNode.h"

GAssignStmt::GAssignStmt(GListNode* name, GListNode* value)
:_value(value),_node(name)
{
  if (_node->_nodetype == NodeType::Literal) {
    _name = GetString(_node);
    _decl = false;
  } else {
    GVariableDecl* decl = (GVariableDecl*)_node->_value;
    _name = decl->name();
    _decl = true;
  }
}

GAssignStmt::~GAssignStmt() {
  FreeNode(_value);
}
