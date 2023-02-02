#include "base/lang/LambdaExpression.h"
#include "base/lang/ASTNode.h"

GLambdaExpression::GLambdaExpression(GListNode* params, GListNode* block)
:_args(params), _states(block) {}

GLambdaExpression::~GLambdaExpression() {
  FreeNode(_states);
  FreeNode(_args);
}