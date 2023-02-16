#include "base/lang/visitor/VariantVisitor.h"
#include "base/lang/visitor/ByteCodeVisitor.h"

VisitFlow GVariantVisitor::apply(GLiteral* node, std::list<NodeType>&) {
  Value arg = getVariant(node);
  int idxParam = addConstant(_compiler->currentChunk(), arg);
  // Note: different with Byte Code visitor, TODO: check and unify them!!
  _compiler->emit((uint8_t)OpCode::OP_CONSTANT, idxParam);
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

VisitFlow GVariantVisitor::apply(GVariableDecl* var, std::list<NodeType>& path) {
  std::string name = var->name();
  _compiler->namedVariant(name, false);
  return VisitFlow::SkipCurrent;
}

Value GVariantVisitor::getVariant(GLiteral* node) {
  if (!node) return Value();
  switch (node->kind()) {
  case AttributeKind::String:
      return node->raw();
  case AttributeKind::Number:
    return atof(node->raw().c_str());
  case AttributeKind::Integer: {
    double v = atof(node->raw().c_str());
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