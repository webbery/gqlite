#pragma once
#include <string>

struct GListNode;

class GWalkDeclaration {
public:
  enum Order {
    VertexEdge,
    EdgeVertex,
  };
  struct WalkElement {
    GListNode* _element;
    WalkElement* _next;
  };

  GWalkDeclaration();
  ~GWalkDeclaration();

  void add(GListNode* node, bool isVertex);

  size_t size() const { return _size; }

  WalkElement* root() const;

  Order order() const { return _order; }
private:
  WalkElement* init(GListNode* node);

private:
  Order _order;
  WalkElement* _walk;
  uint8_t _size;
};

class GEdgeDeclaration {
public:
  /**
  * @param str the describe of direction, such as ->/--/<-
  * @param node if nodetype is Literal, it means a node that point to itself.
  *             Otherwis the node is an edge properties or function that point out the search method
  */
  GEdgeDeclaration(const char* str, GListNode* node);

  GEdgeDeclaration(const char* str, GListNode* from, GListNode* to);

  GEdgeDeclaration(GListNode* edge, GListNode* from, GListNode* to);
  ~GEdgeDeclaration();

  std::string direction() const { return _direction; }
  GListNode* from() const { return _from; }
  GListNode* to() const { return _to; }
  GListNode* value() const;
private:
  GListNode* _from;
  GListNode* _to;
  GListNode* _edge;
  std::string _direction;
};