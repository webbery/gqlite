#include "Graph/BipartiteGraph.h"
#include "SubGraph.h"
#include "Error.h"
#include <deque>

GBipartiteGraph::GBipartiteGraph(const GSubGraph& g): GSubGraph(g) {
  // for (auto ptr: _edges) {
  //     std::string edge = ptr.second->toString();
  //     printf("edge: %s\n", edge.c_str());
  // }
}
void GBipartiteGraph::setLeft(const std::vector<GVertex*>& left) {
  this->_left = left;
  // printf("left: %d\n", _left.size());
}

void GBipartiteGraph::setRight(const std::vector<GVertex*>& right) {
  this->_right = right;
  // printf("right: %d\n", _right.size());
}

Eigen::MatrixXd GBipartiteGraph::toMatrix(const char* weight) {
  Eigen::MatrixXd m;
  m.resize(_left.size(), _right.size());
  for (size_t row = 0; row < _left.size();++row) {
    std::string l = _left[row]->id();
    for (size_t col = 0; col < _right.size(); ++col) {
      std::string r = _right[col]->id();
      edge_id eid(r, l, GraphEdgeDirection::To);
      std::string edge = _edges[eid]->toString();
      int w = _edges[eid]->prop(weight).get<int>();
      // printf("(%d, %d): %d\n", row, col, w);
      m(row, col) = w;
    }
  }
  return m;
}

template <> GBipartiteGraph graph_cast<GBipartiteGraph>(const GSubGraph& sg) {
  GSubGraph& g = const_cast<GSubGraph&>(sg);
  std::vector<GVertex*> left, right;
  std::unordered_map<std::string, short> colored;
  const std::string first = g.vertex_begin()->first;
  colored[first] = COLORED_RED;
  std::deque<std::string> qvertexes;
  qvertexes.push_back(first);
  int n = 1;
  while(qvertexes.size()) {
    std::string& v = qvertexes.front();
    for (auto itr = g[v]->neighbor_begin(), end = g[v]->neighbor_end();
      itr != end; ++itr) {
        std::string neighbor = (*itr)->id();
        // printf("%s's neighbor: %s\n", v.c_str(), neighbor.c_str());
        if (colored.count(neighbor)) {
          if (colored[neighbor] == colored[v]) {
            throw graph_bad_cast("graph is not a bipartite");
          }
        }
        else {
          n += 1;
          colored[neighbor] = -colored[v];
          // printf("add neighbor: %s\n", neighbor.c_str());
          qvertexes.push_back(neighbor);
        }
    }
    qvertexes.pop_front();
  }
  // printf("Size: %d\n", colored.size());
  for (auto& vertex: colored) {
    if (vertex.second == COLORED_RED) left.push_back(g[vertex.first]);
    else right.push_back(g[vertex.first]);
  }

  GBipartiteGraph bg(sg);
  bg.setLeft(left);
  bg.setRight(right);
  return bg;
}