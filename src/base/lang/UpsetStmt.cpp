#include "base/lang/UpsetStmt.h"

GUpsetStmt::GUpsetStmt(const std::string& graph, GASTNode* ast)
:_name(graph)
, _node(ast) {

}