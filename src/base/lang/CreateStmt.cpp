#include "base/lang/CreateStmt.h"
#include "base/lang/ASTNode.h"

GCreateStmt::GCreateStmt(const std::string& name, GASTNode* groups)
: _name(name)
, _groups(groups)
{}

GCreateStmt::~GCreateStmt()
{
  FreeAst(_groups);
}

std::string GCreateStmt::name() const { return _name; }

GASTNode* GCreateStmt::groups() const { return _groups; }
