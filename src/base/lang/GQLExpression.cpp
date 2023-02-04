#include "base/lang/ASTNode.h"
#include "base/lang/GQLExpression.h"
#include "base/lang/ASTNode.h"

GGQLExpression::GGQLExpression(CMDType type, const std::string& params)
  :_cmdType(type)
  ,_params(params)
{
}

GBinaryExpression::GBinaryExpression(Operator op, GListNode* left, GListNode* right)
  :_operator(op)
  ,_left(left)
  ,_right(right) {}

GBinaryExpression::~GBinaryExpression() {
  FreeNode(_left);
  FreeNode(_right);
}