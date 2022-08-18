#include "base/lang/GQLExpression.h"

GGQLExpression::GGQLExpression(CMDType type, const std::string& params)
  :_cmdType(type)
  ,_params(params)
{
}