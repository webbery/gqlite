#include "base/lang/BlockStmt.h"
#include "base/lang/ASTNode.h"

GBlockStmt::~GBlockStmt(){
  FreeNode(_block);
}