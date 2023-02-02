#include "base/lang/QueryStmt.h"
#include "base/lang/ASTNode.h"

GQueryStmt::GQueryStmt(GListNode* query, GListNode* graph, GListNode* conditions)
:_query(query)
,_graph(graph)
,_conditions(conditions){
}

GQueryStmt::~GQueryStmt() {
  FreeNode(_query);
  FreeNode(_graph);
  FreeNode(_conditions);
}