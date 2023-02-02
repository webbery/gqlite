#include "base/lang/GroupStmt.h"
#include "base/lang/ASTNode.h"

GGroupStmt::GGroupStmt(const std::string& name, GListNode* props, GListNode* indexes)
:_name(name)
,_props(props)
,_indexes(indexes) {}

GGroupStmt::~GGroupStmt() {
  FreeNode(_props);
  FreeNode(_indexes);
}

GVertexGroupStmt::GVertexGroupStmt(const std::string& name, GListNode* props, GListNode* indexes)
: GGroupStmt(name, props, indexes)
{
  _type = GroupType::Vertex;
}

GEdgeGroupStmt::GEdgeGroupStmt(const std::string& name, GListNode* props, const std::string& from, const std::string& to)
: GGroupStmt(name, props, nullptr)
, _from(from)
, _to(to) {
  _type = GroupType::Edge;
}