#include "base/lang/ArrayExpression.h"
#include "base/lang/ASTNode.h"

GArrayExpression::GArrayExpression()
{
  
}

GArrayExpression::~GArrayExpression() {
  for (GASTNode* node: _elements) {
    FreeAst(node);
  }
  _elements.clear();
}

void GArrayExpression::addElement(GASTNode* element) {
  _elements.emplace_back(element);
}