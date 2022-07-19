#include "base/lang/GroupStmt.h"
#include "base/lang/ASTNode.h"

GGroupStmt::GGroupStmt(const std::string& name, GASTNode* props)
:_name(name)
,_props(props) {}

GGroupStmt::~GGroupStmt() {
  FreeAst(_props);
}