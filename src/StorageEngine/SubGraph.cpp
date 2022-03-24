#include "SubGraph.h"
#include "gqlite.h"

GSubGraph::~GSubGraph() {}

int GSubGraph::addVertex(const std::string& id, const nlohmann::json& props) {
  GVertex* v = new GVertex();
  v->set(id, props);
  return ECode_Success;
}

int GSubGraph::addEdge(const std::string& from, const std::string& to, GraphEdgeDirection direction, const nlohmann::json& props) {
  return ECode_Success;
}
