#pragma once
#include "base/lang/GQLExpression.h"

class GReturnStmt {
public:
  GReturnStmt(GListNode* value);
  ~GReturnStmt();

  GListNode* expr() { return _expr; }

  uint8_t getOperator() {
    return (uint8_t)OpCode::OP_RETURN;
  }
private:
  GListNode* _expr;
};