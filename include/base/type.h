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
  ArrayExpression,
  ObjectExpression,
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
