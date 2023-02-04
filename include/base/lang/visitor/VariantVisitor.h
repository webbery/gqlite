#pragma once
#include "IVisitor.h"
#include "base/lang/ASTNode.h"
#include "base/lang/Literal.h"

class GVariantVisitor : public GVisitor {
public:
  GVariantVisitor(){}

  virtual VisitFlow apply(GLiteral* node, std::list<NodeType>&) {
    _literal = node;
    return VisitFlow::Return;
  }

  virtual VisitFlow apply(GProperty*, std::list<NodeType>&) {
    return VisitFlow::Children;
  }

  Value getVariant() {
    switch (_literal->kind()) {
    case AttributeKind::String:
        return _literal->raw();
    default:
        return Value();
    }
  }
private:
  GLiteral* _literal;
};