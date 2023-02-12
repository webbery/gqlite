#include "base/lang/visitor/VariantVisitor.h"
#include "base/lang/visitor/ByteCodeVisitor.h"

VisitFlow GVariantVisitor::apply(GLiteral* node, std::list<NodeType>&) {
  _literal = node;
  return VisitFlow::SkipCurrent;
}

VisitFlow GVariantVisitor::apply(GProperty*, std::list<NodeType>&) {
  return VisitFlow::Children;
}

VisitFlow GVariantVisitor::apply(GLambdaExpression* stmt, std::list<NodeType>& path) {
  GByteCodeVisitor visitor(_gvm, _compiler);
  accept(stmt->block(), &visitor, path);
  return VisitFlow::SkipCurrent;
}

Value GVariantVisitor::getVariant() {
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