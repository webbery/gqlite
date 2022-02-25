#pragma once
#include "json.hpp"
#include <set>
#include "Type/Type.h"

class GGraphInterface {
public:
  virtual int queryVertex(std::set<VertexID>& ids,const GConditions& pred) = 0;
  virtual int queryEdge(const nlohmann::json& pred) = 0;
  virtual int walk() = 0;
};