#pragma once
#include <list>
#include <string>

class GVertex;
class GEdge;
class GPath {
public:
  double cost(const std::string& property);

private:
  std::list<GVertex*> _vertexes;
};