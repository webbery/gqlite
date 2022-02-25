#pragma once
#include "base/type.h"

class ASTVisitor {
public:
  virtual void visit(NodeType type, void* value) {}
  virtual ~ASTVisitor() {}
};