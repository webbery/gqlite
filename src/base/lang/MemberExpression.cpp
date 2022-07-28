#include "base/lang/MemberExpression.h"
#include "base/lang/ASTNode.h"

GMemberExpression::GMemberExpression(GASTNode* obj, GASTNode* property, GASTNode* arguments)
  :_obj(obj), _prop(property), _args(arguments)
{

}

GMemberExpression::~GMemberExpression()
{
  FreeAst(_args);
  FreeAst(_prop);
  FreeAst(_obj);
}
