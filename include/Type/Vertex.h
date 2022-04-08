#pragma once
#include "base/Serializer.h"
#include "base/Statement.h"
#include "json.hpp"

typedef std::string GLiteralVertex;

class GAttributeVertex: public GSerializer {
public:
private:
  std::vector<GLiteralVertex> _names;
};

class GEdge;
class GVertex {
public:
  virtual ~GVertex() {}

  std::string id()const { return _id; }
  const nlohmann::json& property() const { return _json; }
  nlohmann::json& property() { return _json; }

  int set(const std::string& id, const nlohmann::json& prop) {
    _id = id;
    _json = prop;
    return 0;
  }

  void addNeighbor(GVertex* neighbor) {
    auto ptr = std::lower_bound(_vertexes.begin(), _vertexes.end(), neighbor);
    if (ptr == _vertexes.end() || *ptr != neighbor) {
      // printf("vertex[%s] add neighbor[%s]\n", _id.c_str(), neighbor->id().c_str());
      _vertexes.insert(ptr, neighbor);
    }
  }

  void addEdge(GEdge* edge) {
    auto ptr = std::lower_bound(_edges.begin(), _edges.end(), edge);
    if (ptr == _edges.end() || *ptr != edge) {
      _edges.insert(ptr, edge);
    }
  }

  void eraseEdge(GEdge* edge);

  typedef std::vector< GEdge* >::const_iterator edge_iterator;
  typedef std::vector< GVertex* >::const_iterator vertex_iterator;

  edge_iterator edge_begin()const {
    return _edges.begin();
  }
  edge_iterator edge_end()const {
    return _edges.end();
  }

  vertex_iterator neighbor_begin()const {
    return _vertexes.begin();
  }
  vertex_iterator neighbor_end() const {
    return _vertexes.end();
  }

protected:
  std::string _id;
  nlohmann::json _json;
  std::vector< GEdge* > _edges;
  std::vector< GVertex* > _vertexes;
};
