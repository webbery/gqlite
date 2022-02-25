#pragma once
#include <string>
#include "json.hpp"
#include "Graph.h"

class GGraph;
struct gast;
namespace upset
{
  std::string exportVertexes(GGraph* g);
  std::string exportEdges(GGraph* g);
} // namespace upset  