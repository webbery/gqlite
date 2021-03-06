#pragma once

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
  ConditionStatement,
  GroupStatement,
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
  VertexDeclaration,
  EdgeDeclaration,
  Identifier,
  Literal,
  BlockStatement,
  ReturnStatement,
  BinaryExpression,
  ArrayExpression,
  ObjectExpression,
  GQLExpression,
  CallExpression,
  MemberExpression,
};

enum AttributeKind {
  Number,
  Integer,
  String,
  Binary,
  Datetime,
};
