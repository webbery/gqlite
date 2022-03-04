#pragma once

enum NodeType {
  Removable,
  // element type
  Number,
  Integer,
  String,
  Binary,
  Vertex,
  Edge,
  From,
  To,
  ArrayExpression,
  ObjectExpression,
  WalkExpression,
  Property,
  // graph managed type
  CreationStatement,
  IndexStatement,
  UpsetStatement,
  RemoveStatement,
  QueryStatement,
  DropStatement,
  // function
  Identifier,
  Literal,
  Variant,
  IfStatement,
  // prediction
  GreaterThan,
  GreaterThanEqual,
  Equal,
  LessThan,
  LessThanEqual
};
