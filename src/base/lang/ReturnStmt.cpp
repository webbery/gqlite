#include "base/lang/ReturnStmt.h"
#include "base/lang/ASTNode.h"

GReturnStmt::GReturnStmt(GListNode* value)
:_expr(value) {
  
}
GReturnStmt::~GReturnStmt(){
  FreeNode(_expr);
}