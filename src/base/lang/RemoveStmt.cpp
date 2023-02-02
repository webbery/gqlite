#include "base/lang/RemoveStmt.h"
#include "base/lang/ASTNode.h"

GRemoveStmt::GRemoveStmt(const std::string& name, GListNode* array)
  :_name(name)
,_array(array) {}

GRemoveStmt::~GRemoveStmt()
{
  FreeNode(_array);
}

GVertexRemoveStmt::GVertexRemoveStmt(const std::string& name, GListNode* array)
: GRemoveStmt(name, array)
{
  _type = Vertex;
}

GEdgeRemoveStmt::GEdgeRemoveStmt(const std::string& name, GListNode* array)
: GRemoveStmt(name, array)
{
  _type = Edge;
}