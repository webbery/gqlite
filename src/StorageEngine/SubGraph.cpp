#include "SubGraph.h"
#include "gqlite.h"
#include <deque>
#include <unordered_map>
#include <Graph/BipartiteGraph.h>
#include <exception>

namespace {
//   bool BFSForBipartite(const std::map<std::string, GVertex*>& vertexes, ) {
// #define COLORED_RED 1
//     std::unordered_map<std::string, short> colored;
//     std::deque<std::string> qvertexes;
//     const std::string& first = vertexes.begin()->first;
//     colored[first] = COLORED_RED;
//     qvertexes.push_back(first);
//     while (qvertexes.size())
//     {
//       std::string& v = qvertexes.front();
//       for (auto itr = vertexes[v]->neighbor_begin(), end = vertexes[v]->neighbor_end();
//         itr != end; ++itr) {
//         std::string neighbor = (*itr)->id();
//         if (colored.count(neighbor)) {
//           if (colored[neighbor] == colored[v]) return false;
//         }
//         else {
//           colored[neighbor] = -colored[v];
//           qvertexes.push_back(neighbor);
//         }
//       }
//       qvertexes.pop_front();
//     }
//     return true;
//   }
}

GSubGraph::GSubGraph(): _cnt(1) {}

GSubGraph::GSubGraph(const GSubGraph& other)
  : _vertexes(other._vertexes)
  , _edges(other._edges)
  , _cnt(other._cnt + 1)
{
}

GSubGraph::GSubGraph(GSubGraph&& other)
  : _vertexes(other._vertexes)
  , _edges(other._edges)
  , _cnt(other._cnt + 1)
{
  other.DeRef();
  other._vertexes.clear();
  other._edges.clear();
}

GSubGraph& GSubGraph::operator = (const GSubGraph& other) {
  if (this != &other) {
    _vertexes = other._vertexes;
    _edges = other._edges;
    _cnt = other._cnt + 1;
  }
  return *this;
}
GSubGraph& GSubGraph::operator = (GSubGraph&& other) {
  if (this != &other) {
    _vertexes = other._vertexes;
    _edges = other._edges;
    _cnt = other.Ref();
    other._vertexes.clear();
    other._edges.clear();
  }
  return *this;
}

GSubGraph::~GSubGraph() {
  DeRef();
}

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
  // printf("from %s to %s\n", from.c_str(), to.c_str());
  if (_edges.count(eid) > 0) {
    _edges[eid]->set(props);
    return ECode_GQL_Edge_Exist;
  }
  GEdge* edge = new GEdge(from, to, direction);
  _edges[eid] = edge;
  // printf("add edge: %s\n", std::string(eid).c_str());
  _edges[eid]->set(props);
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

Eigen::MatrixXd GSubGraph::toMatrix(const GSubGraph& from, const GSubGraph& to, MatrixType type) {
  size_t fcnt = from._vertexes.size();
  size_t tcnt = to._vertexes.size();
  Eigen::MatrixXd m;
  switch (type)
  {
  case Concat:
    break;
  case Extend:
    {
      m = Eigen::MatrixXd::Zero(fcnt + tcnt, tcnt + fcnt);
    }
    break;
  default:
    break;
  }
  return m;
}

Eigen::MatrixXd GSubGraph::toMatrix(const char* weight)
{
  size_t cnt = _vertexes.size();
  Eigen::MatrixXd m = Eigen::MatrixXd::Zero(cnt, cnt);
  std::map<std::string, size_t> keys;
  auto i = _vertexes.begin();
  for (size_t idx = 0; idx < cnt; ++idx,++i) {
    keys[i->first] = idx;
  }
  if (!weight) {
    for (auto itr = _vertexes.begin(), end = _vertexes.end(); itr!=end; ++itr) {
      const std::string& vertex = itr->first;
      for (auto nitr = itr->second->neighbor_begin(), nend = itr->second->neighbor_end(); nitr != nend; ++nitr) {
        m(keys[vertex], keys[(*nitr)->id()]) = 1;
      }
    }
  }
  else {
    for (auto itr = _vertexes.begin(), end = _vertexes.end(); itr!=end; ++itr) {
      const std::string& vertex = itr->first;
      for (auto nitr = itr->second->neighbor_begin(), nend = itr->second->neighbor_end(); nitr != nend; ++nitr) {
        edge_id eid(vertex, (*nitr)->id(), GraphEdgeDirection::Bidrection);
        m(keys[vertex], keys[(*nitr)->id()]) = (double)_edges[eid]->prop(weight);
      }
    }
  }
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

GVertex* GSubGraph::operator[] (const std::string& vertex) {
  return _vertexes.operator[](vertex);
}
