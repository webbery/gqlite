#pragma once

#include "base/visitor.h"

class ASTEdgeUpdateVisitor : public ASTVisitor {
public:
  ~ASTEdgeUpdateVisitor();
  virtual void visit(NodeType type, void* value);

private:
};