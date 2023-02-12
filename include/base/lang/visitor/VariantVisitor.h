#pragma once
#include "IVisitor.h"
#include "base/lang/ASTNode.h"

class GVM;
struct Compiler;
class GVariantVisitor : public GVisitor {
public:
  GVariantVisitor(GVM* gvm = nullptr, Compiler* compiler = nullptr):_literal(nullptr), _gvm(gvm), _compiler(compiler) {}

  virtual VisitFlow apply(GLiteral* node, std::list<NodeType>&);

  virtual VisitFlow apply(GProperty*, std::list<NodeType>&);

  VisitFlow apply(GLambdaExpression* stmt, std::list<NodeType>& path);

  Value getVariant();
private:
  Compiler* _compiler;
  GLiteral* _literal;
  GVM* _gvm;
};