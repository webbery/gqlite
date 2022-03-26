#pragma once
#include "json.hpp"
#include <set>
#include "Type/Type.h"
#include "operand/MatchPattern.h"

enum class GraphValueType {
  Undefined,
  Binary,
  Number,
  String,
  Datetime,
  Vector,
};

struct GraphProperty {
  VertexID _maxDegreeID;
  VertexID _maxInID;
  VertexID _maxOutID;
  uint32_t _vertexCount = 0;
  uint32_t _edgeCount = 0;
  std::string _name;
  /**
   * @brief record value types.
   */
  std::map<std::string, GraphValueType> _types;
  /**
   * @brief query index
   **/
  std::vector<std::string> _indexes;
};

class GGraphInterface {
public:
  // virtual int query(const GMatchPattern& pattern) = 0;

  const GraphProperty& property() const {return _property;}
protected:
  GraphProperty _property;
};