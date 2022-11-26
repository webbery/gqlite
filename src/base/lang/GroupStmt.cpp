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

GVertexGroupStmt::GVertexGroupStmt(const std::string& name, GASTNode* props, GASTNode* indexes)
: GGroupStmt(name, props, indexes)
{
  _type = GroupType::Vertex;
}

GEdgeGroupStmt::GEdgeGroupStmt(const std::string& name, GASTNode* props, const std::string& from, const std::string& to)
: GGroupStmt(name, props, nullptr)
, _from(from)
, _to(to) {
  _type = GroupType::Edge;
}