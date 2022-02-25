#pragma once
#include "IGraph.h"

/*
 * This class is used to store vertexes and edges that for next query.
 */
class GSubGraph : public GGraphInterface {
public:
  virtual int queryVertex(std::set<VertexID>& ids,const GConditions& pred);
  virtual int queryEdge(const nlohmann::json& pred);
  virtual int walk();
};