#include "base/lang/CreateStmt.h"
#include "base/lang/ASTNode.h"

GCreateStmt::GCreateStmt(const std::string& name, GListNode* groups)
: _name(name)
, _groups(groups)
{}

GCreateStmt::~GCreateStmt()
{
  FreeNode(_groups);
}

std::string GCreateStmt::name() const { return _name; }

GListNode* GCreateStmt::groups() const { return _groups; }
