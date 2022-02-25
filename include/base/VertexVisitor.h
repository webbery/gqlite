#pragma once
#include "base/visitor.h"
#include "Type/QueryCondition.h"

class ASTVertexUpdateVisitor : public ASTVisitor {
public:
  ~ASTVertexUpdateVisitor();
  virtual void visit(NodeType type, void* value);

private:
};

class ASTVertexQueryVisitor : public ASTVisitor {
public:
  ~ASTVertexQueryVisitor();

  virtual void visit(NodeType, void* value);

  const GConditions& conditions()  { return _preds; }
private:
  GConditions _preds;
};