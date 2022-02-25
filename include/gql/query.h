#pragma once
#include <vector>
#include <set>
#include "gqlite.h"
#include "Graph.h"

struct gql_node;
namespace query
{
  void get_vertexes(GGraph* pGraph, const std::vector<VertexID>& ids, gqlite_result& results);
  void get_vertexes(GGraph* pGraph, const std::set<VertexID>& ids, gqlite_result& results);
  void filter_property(gqlite_result& results, gql_node* properties);
  void release_vertexes(gqlite_result& results);
} // namespace query
