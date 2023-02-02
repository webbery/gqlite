#include "base/lang/UpsetStmt.h"
#include "base/lang/ASTNode.h"

GUpsetStmt::GUpsetStmt(const std::string& graph, GListNode* ast)
:_name(graph)
, _node(ast)
, _conditions(nullptr) {

}


GUpsetStmt::GUpsetStmt(const std::string& vertexGroup, GListNode* ast, GListNode* conditions)
  :_name(vertexGroup)
  , _node(ast)
, _conditions(conditions) {

}

GUpsetStmt::~GUpsetStmt()
{
  FreeNode(_node);
  if (_conditions) FreeNode(_conditions);
}
