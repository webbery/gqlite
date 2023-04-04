#pragma once
#include <cstdint>
#include <string>

#define OBJECT_TYPE_NAME  "_obj_t"
typedef double REAL;

enum NodeType {
  InvalidNode,
  Removable,
  WalkExpression,
  Property,
  // graph managed type
  CreationStatement,
  IndexStatement,
  UpsetStatement,
  RemoveStatement,
  QueryStatement,
  DropStatement,
  DumpStatement,
  ConditionStatement,
  GroupStatement,
  // Language
  Program,
  FunctionDeclaration,
  VariableDeclaration,
  VariableDeclarator,
  VertexDeclaration,
  EdgeDeclaration,
  WalkDeclaration,
  Identifier,
  Literal,
  BlockStatement,
  IfStatement,
  ReturnStatement,
  AssignStatement,
  BinaryExpression,
  ArrayExpression,
  ObjectExpression,
  LambdaExpression,
  GQLExpression,
  CallExpression,
  MemberExpression,
};

enum class AttributeKind {
  Number,
  Integer,
  String,
  Binary,
  Datetime,
  Vector,
};

using group_t = uint8_t;
using node_t = uint64_t;
using edge2_t = std::string;
using attr_t = uint8_t;
