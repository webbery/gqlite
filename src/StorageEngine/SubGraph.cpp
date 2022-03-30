#include "SubGraph.h"
#include "gqlite.h"
#include <deque>
#include <unordered_map>

GSubGraph::~GSubGraph() {}

int GSubGraph::addVertex(const std::string& id, const nlohmann::json& props) {
  if (_vertexes.count(id) > 0) {
    _vertexes[id]->set(id, props);
    return ECode_GQL_Vertex_Exist;
  }
  GVertex* v = new GVertex();
  v->set(id, props);
  _vertexes[id] = v;
  return ECode_Success;
}

int GSubGraph::addEdge(const std::string& from, const std::string& to, GraphEdgeDirection direction, const nlohmann::json& props) {
  edge_id eid(from, to, direction);
  if (_edges.count(eid) > 0) {
    _edges[eid]->set(props);
    return ECode_GQL_Edge_Exist;
  }
  GEdge* edge = new GEdge(from, to, direction);
  _edges[eid] = edge;
  _vertexes[from]->addEdge(edge);
  _vertexes[from]->addNeighbor(_vertexes[to]);
  _vertexes[to]->addEdge(edge);
  _vertexes[to]->addNeighbor(_vertexes[from]);
  return ECode_Success;
}

int GSubGraph::eraseVertex(const std::string& id)
{
  return ECode_Success;
}

int GSubGraph::earseEdge(const edge_id& id)
{
  return ECode_Success;
}

bool GSubGraph::isBipartite()
{
  if (_vertexes.size() == 0) return true;
  // 0 means uninitialized, 1 means red and -1 means green
#define COLORED_RED 1
  std::unordered_map<std::string, short> colored;
  std::deque<std::string> qvertexes;
  const std::string& first = _vertexes.begin()->first;
  colored[first] = COLORED_RED;
  qvertexes.push_back(first);
  while (qvertexes.size())
  {
    std::string& v = qvertexes.front();
    for (auto itr = _vertexes[v]->neighbor_begin(), end = _vertexes[v]->neighbor_end();
      itr != end; ++itr) {
      std::string neighbor = (*itr)->id();
      if (colored.count(neighbor)) {
        if (colored[neighbor] == colored[v]) return false;
      }
      else {
        colored[neighbor] = -colored[v];
        qvertexes.push_back(neighbor);
      }
    }
    qvertexes.pop_front();
  }
  return true;
}

Eigen::MatrixXd GSubGraph::toMatrix()
{
  Eigen::MatrixXd m;
  return m;
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
