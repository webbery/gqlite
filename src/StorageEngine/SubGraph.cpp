#include "SubGraph.h"
#include "gqlite.h"

GSubGraph::~GSubGraph() {}

int GSubGraph::addVertex(const std::string& id, const nlohmann::json& props) {
  if (_vertexes.count(id) > 0) {
    delete _vertexes[id];
  }
  GVertex* v = new GVertex();
  v->set(id, props);
  _vertexes[id] = v;
  return ECode_Success;
}

int GSubGraph::addEdge(const std::string& from, const std::string& to, GraphEdgeDirection direction, const nlohmann::json& props) {
  edge_id eid(from, to, direction);
  if (_edges.count(eid) > 0) {
    delete _edges[eid];
  }
  GEdge* edge = new GEdge(from, to, direction);
  _edges[eid] = edge;
  return ECode_Success;
}

bool GSubGraph::operator==(const GSubGraph& other)
{
  if (_vertexes.size() != other._vertexes.size() || _edges.size() != other._edges.size()) return false;
  auto vitr = _vertexes.begin();
  auto ovitr = other._vertexes.begin();
  for (; vitr != _vertexes.end(); ++vitr, ++ovitr) {
    // is vertex name equal or not
    if (vitr->first != ovitr->first) return false;
  }
  auto eitr = _edges.begin();
  auto oeitr = other._edges.begin();
  for (; eitr != _edges.end(); ++eitr, ++oeitr) {
    if (eitr->first != oeitr->first) return false;
  }
  return true;
}
