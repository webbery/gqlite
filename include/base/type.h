#pragma once
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
  // prediction
  GreaterThan,
  GreaterThanEqual,
  Equal,
  LessThan,
  LessThanEqual,
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
