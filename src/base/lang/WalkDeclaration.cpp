#include "base/lang/WalkDeclaration.h"
#include "base/lang/ASTNode.h"

GWalkDeclaration::GWalkDeclaration()
  :_walk(nullptr)
  , _size(0)
{

}

GWalkDeclaration::~GWalkDeclaration()
{
  auto ptr = _walk;
  while (ptr) {
    FreeNode(ptr->_element);
    auto next = ptr->_next;
    free(ptr);
    ptr = next;
  }
}

void GWalkDeclaration::add(GListNode* node, bool isVertex)
{
  if (!_walk) {
    if (isVertex) _order = VertexEdge;
    else _order = EdgeVertex;

    _walk = init(node);
    return;
  }
  auto ptr = _walk;
  auto last = _walk;
  while (ptr) {
    last = ptr;
    ptr = ptr->_next;
  }
  last->_next = init(node);
}


GWalkDeclaration::WalkElement* GWalkDeclaration::root() const
{
  return _walk;
}

GWalkDeclaration::WalkElement* GWalkDeclaration::init(GListNode* node)
{
  auto ptr = (WalkElement*)malloc(sizeof(WalkElement));
  ptr->_next = nullptr;
  ptr->_element = node;
  _size += 1;
  return ptr;
}

GEdgeDeclaration::GEdgeDeclaration(const char* str, GListNode* node)
  :_from(nullptr),_to(nullptr), _edge(nullptr)
  ,_direction(str)
{
  if (node->_nodetype == NodeType::Literal) {
    _from = node;
    _to = node;
  }
  else {
    _edge = node;
  }
}

GEdgeDeclaration::GEdgeDeclaration(const char* str, GListNode* from, GListNode* to)
  :_from(from),_to(to), _edge(nullptr)
  , _direction(str)
{

}

GEdgeDeclaration::GEdgeDeclaration(GListNode* connection, GListNode* from, GListNode* to)
  : _from(from), _to(to), _edge(connection)
{
  if (connection->_nodetype == NodeType::Literal) {
    _direction = GetString(connection);
  }
  else {
    GEdgeDeclaration* edge = (GEdgeDeclaration*)connection->_value;
    _direction = edge->direction();
  }
}

GEdgeDeclaration::~GEdgeDeclaration()
{
  if (_from != _to) {
    FreeNode(_from);
  }
  FreeNode(_to);
  FreeNode(_edge);
}

GListNode* GEdgeDeclaration::value() const
{
  if (!_edge) return nullptr;
  if (_edge->_nodetype == EdgeDeclaration) {
    GEdgeDeclaration* edge = (GEdgeDeclaration*)_edge->_value;
    return edge ? edge->from() : nullptr;
  }
  return _edge;
}
