#pragma once
#include <vector>
#include "Type/Vertex.h"
#include "Type/Edge.h"
#include "IGraph.h"

class GSubGraph : public GGraphInterface {
public:
  virtual ~GSubGraph();

  virtual int addVertex(const std::string& id, const nlohmann::json& props);
  virtual int addEdge(const std::string& from, const std::string& to, GraphEdgeDirection direction, const nlohmann::json& props);

private:
  std::vector<GVertex*> _vertexes;
  std::vector<GEdge*> _edges;
  std::vector<GLink> _links;
};
