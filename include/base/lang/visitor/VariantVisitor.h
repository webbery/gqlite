#pragma once
#include "IVisitor.h"
#include "base/lang/ASTNode.h"
#include "base/lang/Literal.h"

class GVariantVisitor : public GVisitor {
public:
  GVariantVisitor():_literal(nullptr) {}

  virtual VisitFlow apply(GLiteral* node, std::list<NodeType>&) {
    _literal = node;
    return VisitFlow::SkipCurrent;
  }

  virtual VisitFlow apply(GProperty*, std::list<NodeType>&) {
    return VisitFlow::Children;
  }

  Value getVariant() {
    if (!_literal) return Value();
    switch (_literal->kind()) {
    case AttributeKind::String:
        return _literal->raw();
    case AttributeKind::Number:
      return atof(_literal->raw().c_str());
    case AttributeKind::Integer: {
      double v = atof(_literal->raw().c_str());
      if (v <= std::numeric_limits<int>::max()) {
        return (int)v;
      }
      else if (v <= std::numeric_limits<long>::max()) {
        return (long)v;
      }
      else if (v > 0 && v <= std::numeric_limits<uint64_t>::max()) {
        return (uint64_t)v;
      }
      return v;
    }
    default:
        return Value();
    }
  }
private:
  GLiteral* _literal;
};