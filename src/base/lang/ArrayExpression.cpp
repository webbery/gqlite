#include "base/lang/ArrayExpression.h"
#include "base/lang/ASTNode.h"

GArrayExpression::GArrayExpression()
  :_type(ElementType::Undefined)
{
  
}

GArrayExpression::~GArrayExpression() {
  for (GASTNode* node: _elements) {
    FreeAst(node);
  }
  _elements.clear();
}

void GArrayExpression::addElement(GASTNode* element) {
  if (isBasicElement(element)) {

  }
  _elements.emplace_back(element);
}

bool GArrayExpression::isBasicElement(GASTNode*)
{
  return false;
}
