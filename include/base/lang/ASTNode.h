#pragma once
#include "../type.h"
#include "base/lang/lang.h"
#include <stddef.h>
#include <list>
#include "Graph/GRAD.h"

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

struct GASTNode {
  enum NodeType _nodetype;
  void* _value;
  struct GASTNode* _children;
  size_t _size;
};

GASTNode* NewAst(enum NodeType type, void* value, GASTNode* children, size_t size);
void FreeAst(GASTNode* root);
void DumpAst(GASTNode* root, int level = 0);

GASTNode* LoadAST();

/****
 * 
 */
std::string GetString(GASTNode* node);
attribute_t GetLiteral(GASTNode* node);
std::vector<double> GetVector(GASTNode* node);

class GViewVisitor {
public:
  VisitFlow apply(GASTNode* stmt, std::list<NodeType>& path);
  VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GGQLExpression* stmt, std::list<NodeType>& path);
  VisitFlow apply(GProperty* stmt, std::list<NodeType>& path);
  VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path);
  VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path);
  VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path);
  VisitFlow apply(GWalkDeclaration* stmt, std::list<NodeType>& path);
  VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GObjectFunction* stmt, std::list<NodeType>& path);
  VisitFlow apply(GDumpStmt* stmt, std::list<NodeType>& path) {
    return VisitFlow::SkipCurrent;
  }
  VisitFlow apply(GRemoveStmt* stmt, std::list<NodeType>& path) {
    return VisitFlow::Return;
  }
  VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path);

private:
};
