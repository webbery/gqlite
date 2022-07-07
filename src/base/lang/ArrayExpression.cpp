#include "base/lang/ArrayExpression.h"
#include "base/lang/ASTNode.h"

GArrayExpression::GArrayExpression()
{
  
}

void GArrayExpression::addElement(GASTNode* element) {
  _elements.emplace_back(element);
}