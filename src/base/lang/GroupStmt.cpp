#include "base/lang/GroupStmt.h"
#include "base/lang/ASTNode.h"

GGroupStmt::GGroupStmt(const std::string& name, GASTNode* props, GASTNode* indexes)
:_name(name)
,_props(props)
,_indexes(indexes) {}

GGroupStmt::~GGroupStmt() {
  FreeAst(_props);
  FreeAst(_indexes);
}