#include "base/lang/QueryStmt.h"

GQueryStmt::GQueryStmt(GASTNode* query, GASTNode* graph, GASTNode* conditions)
:_query(query)
,_graph(graph)
,_conditions(conditions){}