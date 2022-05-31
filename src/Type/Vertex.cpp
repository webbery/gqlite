#include "Type/Vertex.h"
#include "Type/Edge.h"

void GVertex::eraseEdge(GEdge* edge)
{
  auto ptr = std::lower_bound(_edges.begin(), _edges.end(), edge);
  if (ptr != _edges.end() && *ptr == edge) {
    _edges.erase(ptr);
  }
  std::string rmVertex = edge->from();
  if (rmVertex == _id) {
    rmVertex = edge->to();
  }
  for (auto itr = _vertexes.begin(), end = _vertexes.end(); itr != end; ++itr) {
    if ((*itr)->id() == rmVertex) {
      _vertexes.erase(itr);
      break;
    }
  }
}

bool GVertex::addChild(GVertex* pVertex) {
  return false;
}

