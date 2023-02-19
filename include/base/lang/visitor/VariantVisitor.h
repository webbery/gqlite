#pragma once
#include "IVisitor.h"
#include "base/lang/ASTNode.h"

class GVM;
struct Compiler;
class GVariantVisitor : public GVisitor {
public:
  GVariantVisitor(GVM* gvm = nullptr, Compiler* compiler = nullptr):_gvm(gvm), _compiler(compiler) {}

  virtual VisitFlow apply(GLiteral* node, std::list<NodeType>&);

  virtual VisitFlow apply(GProperty*, std::list<NodeType>&);

  VisitFlow apply(GLambdaExpression* stmt, std::list<NodeType>& path);

  VisitFlow apply(GVariableDecl* var, std::list<NodeType>& path);

  VisitFlow apply(GBinaryExpression* stmt, std::list<NodeType>& path);

  Value getVariant(GLiteral* node);
private:
  Compiler* _compiler;
  GVM* _gvm;
};