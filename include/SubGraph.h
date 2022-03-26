#pragma once
#include <vector>
#include "Type/Vertex.h"
#include "Type/Edge.h"
#include "IGraph.h"
#include <map>

class GSubGraph : public GGraphInterface {
public:
  virtual ~GSubGraph();

  virtual int addVertex(const std::string& id, const nlohmann::json& props = nlohmann::json());
  virtual int addEdge(const std::string& from, const std::string& to, GraphEdgeDirection direction, const nlohmann::json& props = nlohmann::json());

  bool operator == (const GSubGraph& other);
private:
  std::map<std::string, GVertex*> _vertexes;
  std::map<edge_id, GEdge*> _edges;
};

