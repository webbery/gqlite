#include "base/lang/UpsetStmt.h"
#include "base/lang/ASTNode.h"

GUpsetStmt::GUpsetStmt(const std::string& graph, GASTNode* ast)
:_name(graph)
, _node(ast) {

}

GUpsetStmt::~GUpsetStmt()
{
  FreeAst(_node);
}
