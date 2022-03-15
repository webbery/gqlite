#pragma once

#ifdef EMBEDING
typedef uint16_t EdgeID;
typedef uint32_t VertexID;
#else
#include <string>
typedef std::string EdgeID;
typedef std::string VertexID;
#endif
#include <any>
#include "base/type.h"

struct GPredition {
  // function type such as gte/lt...
  std::string _fn;
  // index name
  std::string _indx;
  // type of value
  NodeType _type;
  std::any _value;
  GPredition* _next;
};

struct GVertexCondition {
  bool _isAnd;    // is and/or
  GPredition* _preds;
  GVertexCondition* _next;
};

struct GEdgeCondition {
  GPredition* _preds;
  GEdgeCondition* _next;
};

struct GWalkExpr {
  // property of edge that will be used by expression
  std::string _props;
  // function type such as Astar/dijk..., or expression that can be envoked.
  std::string _fn;
  // if _isFunc is true, _fn is a function string. Other wise _fn is a normal string that is build in.
  bool _isFunc;
};

enum ConditionType {
  CondType_Vertex,
  CondType_Edge,
  CondType_Walk,
};

struct GConditions {
  GVertexCondition* _vertex_condition;
  GEdgeCondition* _edge_condition;
  GWalkExpr* _walk_expression;
};

#include "Binary.h"
#include "Datetime.h"
#include "Hash.h"
#include "Integer.h"
#include "Real.h"
#include "Text.h"
#include "Vector.h"