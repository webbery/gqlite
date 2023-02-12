#include "base/lang/MemberExpression.h"
#include "base/lang/ASTNode.h"

GMemberExpression::GMemberExpression(GListNode* obj, GListNode* property, GListNode* arguments)
  :_obj(obj), _prop(property), _args(arguments)
{

}

GMemberExpression::~GMemberExpression()
{
  FreeNode(_args);
  FreeNode(_prop);
  FreeNode(_obj);
}

std::string GMemberExpression::GetObjectName() const
{
  return GetString(_obj);
}

std::string GMemberExpression::GetPropertyName() const {
  return GetString(_prop);
}
