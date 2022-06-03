#include "base/lang/ArrayExpression.h"
#include "base/lang/ASTNode.h"

GArrayExpression::GArrayExpression(GASTNode* elements, size_t len)
: _elements(len){
  for (size_t idx = 0; idx < len; ++idx) {
    _elements[idx] = elements + idx;
  }
}
