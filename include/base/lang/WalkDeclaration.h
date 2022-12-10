#pragma once
#include <string>

struct GASTNode;

class GWalkDeclaration {
public:
  enum Order {
    VertexEdge,
    EdgeVertex,
  };
  struct WalkElement {
    GASTNode* _element;
    WalkElement* _next;
  };

  GWalkDeclaration();
  ~GWalkDeclaration();

  void add(GASTNode* node, bool isVertex);

  size_t size() const { return _size; }

  WalkElement* root() const;

  Order order() const { return _order; }
private:
  WalkElement* init(GASTNode* node);

private:
  Order _order;
  WalkElement* _walk;
  uint8_t _size;
};

class GEdgeDeclaration {
public:
  GEdgeDeclaration(const char* str, GASTNode* node);
  GEdgeDeclaration(const char* str, GASTNode* from, GASTNode* to);
  GEdgeDeclaration(GASTNode* edge, GASTNode* from, GASTNode* to);
  ~GEdgeDeclaration();

  std::string direction() const { return _direction; }
  GASTNode* from() const { return _from; }
  GASTNode* to() const { return _to; }
  GASTNode* value() const;
private:
  GASTNode* _from;
  GASTNode* _to;
  GASTNode* _edge;
  std::string _direction;
};