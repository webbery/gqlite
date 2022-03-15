#pragma once
#include "IGraph.h"

/*
 * This class is used to store vertexes and edges that for next query.
 */
class GSubGraph : public GGraphInterface {
public:
  virtual int query(gqlite_node*& nodes, const GConditions& pred);
};