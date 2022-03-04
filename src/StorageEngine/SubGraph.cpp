#include "SubGraph.h"
#include "gqlite.h"

int GSubGraph::queryVertex(std::set<VertexID>& ids, const GConditions& pred)
{
  return ECode_Success;
}

int GSubGraph::queryEdge(const nlohmann::json& pred)
{
  return ECode_Success;
}

int GSubGraph::walk()
{
  return ECode_Success;
}
