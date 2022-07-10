#include "base/lang/CreateStmt.h"
#include "base/lang/ASTNode.h"

GCreateStmt::GCreateStmt(const std::string& name, GASTNode* indexes)
: _name(name)
, _indexes(indexes) {}

GCreateStmt::~GCreateStmt()
{
  GASTNode* ptr = _indexes;
  while (ptr)
  {
    GASTNode* temp = ptr;
    ptr = ptr->_children;
    FreeAst(temp);
  }
}

std::string GCreateStmt::name() const { return _name; }

GASTNode* GCreateStmt::indexes() const { return _indexes; }