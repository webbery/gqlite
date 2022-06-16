#pragma once
#include "../type.h"
#include "base/lang/lang.h"
#include <stddef.h>

template <NodeType T> struct GTypeTraits {};

template <> struct GTypeTraits<NodeType::GQLExpression> {
  typedef GGQLExpression type;
};
template <> struct GTypeTraits<NodeType::Literal> {
  typedef GLiteral type;
};

template <> struct GTypeTraits<NodeType::CreationStatement> {
  typedef GCreateStmt type;
};

template <> struct GTypeTraits<NodeType::UpsetStatement> {
  typedef GUpsetStmt type;
};

template <> struct GTypeTraits<NodeType::ArrayExpression> {
  typedef GASTNode type;
};

template <> struct GTypeTraits<NodeType::Property> {
  typedef GProperty type;
};

template <> struct GTypeTraits<NodeType::BinaryExpression> {
  typedef GProperty type;
};

template <> struct GTypeTraits<NodeType::QueryStatement> {
  typedef GQueryStmt type;
};

template <> struct GTypeTraits<NodeType::VertexDeclaration> {
  typedef GVertexDeclaration type;
};

template <> struct GTypeTraits<NodeType::EdgeDeclaration> {
  typedef GEdgeDeclaration type;
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

GASTNode* ListJoin(GASTNode* first, GASTNode* second);

/****
 * 
 */
std::string GetString(GASTNode* node);