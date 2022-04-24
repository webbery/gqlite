#pragma once
#include <string>
#include "json.hpp"
#include "Graph.h"

class GGraphInstance;
struct gast;
namespace upset
{
  std::string exportVertexes(GGraphInstance* g);
  std::string exportEdges(GGraphInstance* g);
} // namespace upset  