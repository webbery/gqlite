#pragma once
#include "base/lang/ASTNode.h"
#include "base/lang/GQLExpression.h"
#include "base/lang/lang.h"
#include "base/type.h"
#include <list>
#include <functional>

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

template<> struct GTypeTraits<NodeType::DropStatement> {
  typedef GDropStmt type;
};

template <> struct GTypeTraits<NodeType::UpsetStatement> {
  typedef GUpsetStmt type;
};

template <> struct GTypeTraits<NodeType::ArrayExpression> {
  typedef GArrayExpression type;
};

template <> struct GTypeTraits<NodeType::Property> {
  typedef GProperty type;
};

template <> struct GTypeTraits<NodeType::BinaryExpression> {
  typedef GBinaryExpression type;
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

template <> struct GTypeTraits<NodeType::WalkDeclaration> {
  typedef GWalkDeclaration type;
};

template <> struct GTypeTraits<NodeType::RemoveStatement> {
  typedef GRemoveStmt type;
};

template <> struct GTypeTraits<NodeType::ObjectExpression> {
  typedef GProperty type;
};

template <> struct GTypeTraits<NodeType::GroupStatement> {
  typedef GGroupStmt type;
};

template <> struct GTypeTraits<NodeType::MemberExpression> {
  typedef GMemberExpression type;
};

template <> struct GTypeTraits<NodeType::DumpStatement> {
  typedef GDumpStmt type;
};

template <> struct GTypeTraits<NodeType::CallExpression> {
  typedef GObjectFunction type;
};

template <> struct GTypeTraits<NodeType::LambdaExpression> {
  typedef GLambdaExpression type;
};

template <> struct GTypeTraits<NodeType::ReturnStatement> {
  typedef GReturnStmt type;
};

template <> struct GTypeTraits<NodeType::BlockStatement> {
  typedef GBlockStmt type;
};

template <> struct GTypeTraits<NodeType::VariableDeclaration> {
  typedef GVariableDecl type;
};

/******************************
 * accept function will visit all nodes start from input node.
 * We define ourself's visitor to process nodes when type is matched.
 * And parameter of path show current tree path of node type.
 ******************************/
VisitFlow accept(GListNode* node, GVisitor* visitor, std::list<NodeType>& path);
