#pragma once

#include "base/lang/ObjectFunction.h"
#include "base/lang/VariableDecl.h"
#include "base/lang/visitor/IVisitor.h"
#include "base/gvm/Compiler.h"
#include "base/lang/AST.h"
#include "base/type.h"

class GVM;
class GByteCodeVisitor: public GVisitor {
public:
  Compiler* _compiler;

  GByteCodeVisitor(GVM* gvm, Compiler* compiler = nullptr);

  VisitFlow apply(GReturnStmt* stmt, std::list<NodeType>& path);

  VisitFlow apply(GBlockStmt* stmt, std::list<NodeType>& path);

  VisitFlow apply(GBinaryExpression* stmt, std::list<NodeType>& path);

  VisitFlow apply(GAssignStmt* stmt, std::list<NodeType>& path);

  VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path);

  VisitFlow apply(GObjectFunction* func, std::list<NodeType>& path);

  VisitFlow apply(GVariableDecl* var, std::list<NodeType>& path);

private:
  GVM* _gvm;
  /**
   * Check if assigning.
   */
  bool _assigning;
};