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
  std::shared_ptr < GPredition> _next;
};

struct GWalkExpr {
  // property of edge that will be used by expression
  std::vector<std::string> _props;
  // function type such as Astar/dijk..., or expression that can be envoked.
  std::string _fn;
  // if _isFunc is true, _fn is a function string. Other wise _fn is a normal string that is build in.
  bool _isFunc;
};

struct GConditions {
  bool _isAnd;    // is and/or
  std::shared_ptr<GPredition> _preds;
  std::shared_ptr<GWalkExpr> _walks;
  std::shared_ptr <GConditions> _next;
};

#include "Binary.h"
#include "Datetime.h"
#include "Hash.h"
#include "Integer.h"
#include "Real.h"
#include "Text.h"
#include "Vector.h"