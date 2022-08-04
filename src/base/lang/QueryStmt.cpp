#include "base/lang/QueryStmt.h"
#include "base/lang/ASTNode.h"

GQueryStmt::GQueryStmt(GASTNode* query, GASTNode* graph, GASTNode* conditions)
:_query(query)
,_graph(graph)
,_conditions(conditions){
}

GQueryStmt::~GQueryStmt() {
  FreeAst(_query);
  FreeAst(_graph);
  FreeAst(_conditions);
}