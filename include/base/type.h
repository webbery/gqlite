#pragma once

typedef double REAL;

enum NodeType {
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
  ConditionStatement,
  IfStatement,
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
  Identifier,
  Literal,
  BlockStatement,
  ReturnStatement,
  BinaryExpression,
  ArrayExpression,
  ObjectExpression,
  GQLExpression,
  CallExpression,
};

enum PropertyKind {
  Number,
  Integer,
  String,
  Binary,
  Vertex,
  Edge,
  From,
  To,
};