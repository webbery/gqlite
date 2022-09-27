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
    GLiteral* literal = (GLiteral*)element->_value;
    switch (literal->kind()) {
    case AttributeKind::Number:
      _type = ElementType::Number;
      break;
    case AttributeKind::Integer:
      _type = ElementType::Integer;
      break;
    case AttributeKind::String:
      _type = ElementType::String;
      break;
    default:
      _type = ElementType::Mixed;
      break;
    }
  }
  _elements.emplace_back(element);
}

bool GArrayExpression::isBasicElement(GASTNode* node)
{
  if (node->_nodetype == NodeType::Literal) {
    return true;
  }
  return false;
}
