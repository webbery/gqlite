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

struct GConditions {
  bool _isAnd;    // is and/or
  std::shared_ptr<GPredition> _preds;
  std::shared_ptr < GConditions> _next;
};

#include "Binary.h"
#include "Datetime.h"
#include "Hash.h"
#include "Integer.h"
#include "Real.h"
#include "Text.h"
#include "Vector.h"