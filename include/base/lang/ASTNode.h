#pragma once
#include "../type.h"
#include "base/lang/AssignStmt.h"
#include "base/lang/Literal.h"
#include "base/lang/lang.h"
#include <stddef.h>
#include <list>
#include "Graph/GRAD.h"
#include "visitor/IVisitor.h"

struct GListNode {
  enum NodeType _nodetype;
  void* _value;
  struct GListNode* _children;
};

GListNode* MakeNode(enum NodeType type, void* value, GListNode* children);
void FreeNode(GListNode* root);
void DumpAst(GListNode* root, int level = 0);

/****
 * 
 */
std::string GetString(GListNode* node);
template<typename T>
bool GetLiteral(GLiteral* literal, T& value) {
  switch (literal->kind()) {
  case AttributeKind::String:
    value = literal->raw();
    return true;
  case AttributeKind::Datetime:
    value = (double)atoll(literal->raw().c_str());
    return true;
  case AttributeKind::Integer:
  case AttributeKind::Number:
    value = (double)atof(literal->raw().c_str());
    return true;
  default:
    break;
  }
  return false;
}

template<typename T>
bool GetLiteral(GListNode* node, T& value) {
  if (node->_nodetype != NodeType::Literal) return false;
  GLiteral* literal = (GLiteral*)node->_value;
  return GetLiteral(literal, value);
}
std::vector<double> GetVector(GListNode* node);

class GViewVisitor : public GVisitor {
public:
  VisitFlow apply(GListNode* stmt, std::list<NodeType>& path);
  VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GGQLExpression* stmt, std::list<NodeType>& path);
  VisitFlow apply(GProperty* stmt, std::list<NodeType>& path);
  VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path);
  VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path);
  VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path);
  VisitFlow apply(GWalkDeclaration* stmt, std::list<NodeType>& path) {
    return VisitFlow::Return;
  }
  VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GObjectFunction* stmt, std::list<NodeType>& path);
  VisitFlow apply(GDumpStmt* stmt, std::list<NodeType>& path) {
    return VisitFlow::SkipCurrent;
  }
  VisitFlow apply(GRemoveStmt* stmt, std::list<NodeType>& path) {
    return VisitFlow::Return;
  }
  VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path);
  VisitFlow apply(GLambdaExpression* stmt, std::list<NodeType>& path);
  VisitFlow apply(GReturnStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GBinaryExpression* stmt, std::list<NodeType>& path);
  VisitFlow apply(GBlockStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GVariableDecl* stmt, std::list<NodeType>& path);
  VisitFlow apply(GAssignStmt* stmt, std::list<NodeType>& path);

private:
};
