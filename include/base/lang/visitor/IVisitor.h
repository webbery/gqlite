#pragma once
#include <list>
#include "base/type.h"

/*************************
 * AST visitor flow control.
 * skip will not process current node.
 * return will stop recursive ast node.
 * children will visit its children.
 *************************/
enum class VisitFlow {
  SkipCurrent,
  Return,
  Children
};

struct GListNode;
class GGQLExpression;
class GLiteral;
class GVariableDecl;
class GBlockStmt;
class GLambdaExpression;
class GObjectFunction;
class GDumpStmt;
class GMemberExpression;
class GGroupStmt;
class GProperty;
class GRemoveStmt;
class GWalkDeclaration;
class GEdgeDeclaration;
class GVertexDeclaration;
class GQueryStmt;
class GBinaryExpression;
class GArrayExpression;
class GDropStmt;
class GUpsetStmt;
class GCreateStmt;
class GReturnStmt;
class GAssignStmt;

class GVisitor {
public:
  virtual VisitFlow apply(GGQLExpression*, std::list<NodeType>&){ return VisitFlow::Children; }
  virtual VisitFlow apply(GListNode*, std::list<NodeType>&)     { return VisitFlow::Children; }
  virtual VisitFlow apply(GLiteral*, std::list<NodeType>&)      { return VisitFlow::Children; }
  virtual VisitFlow apply(GProperty*, std::list<NodeType>&)     { return VisitFlow::Children; }
  virtual VisitFlow apply(GArrayExpression*, std::list<NodeType>&) { return VisitFlow::Children; }
  virtual VisitFlow apply(GVariableDecl*, std::list<NodeType>&) { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GBlockStmt*, std::list<NodeType>&)    { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GLambdaExpression*, std::list<NodeType>&) { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GObjectFunction*, std::list<NodeType>&)   { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GDumpStmt*, std::list<NodeType>&)     { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GMemberExpression*, std::list<NodeType>&) { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GGroupStmt*, std::list<NodeType>&)    { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GRemoveStmt*, std::list<NodeType>&)   { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GWalkDeclaration*, std::list<NodeType>&) { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GEdgeDeclaration*, std::list<NodeType>&) { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GVertexDeclaration*, std::list<NodeType>&) { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GQueryStmt*, std::list<NodeType>&) { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GBinaryExpression*, std::list<NodeType>&) { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GUpsetStmt*, std::list<NodeType>&)    { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GDropStmt*, std::list<NodeType>&)     { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GCreateStmt*, std::list<NodeType>&)   { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GReturnStmt*, std::list<NodeType>&)   { return VisitFlow::SkipCurrent; }
  virtual VisitFlow apply(GAssignStmt*, std::list<NodeType>&)   { return VisitFlow::SkipCurrent; }
};