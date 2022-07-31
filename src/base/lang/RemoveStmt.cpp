#include "base/lang/RemoveStmt.h"
#include "base/lang/ASTNode.h"

GRemoveStmt::GRemoveStmt(const std::string& name, GASTNode* array)
  :_name(name)
,_array(array) {}

GRemoveStmt::~GRemoveStmt()
{
  FreeAst(_array);
}
