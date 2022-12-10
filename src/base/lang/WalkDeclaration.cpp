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
    FreeAst(ptr->_element);
    auto next = ptr->_next;
    free(ptr);
    ptr = next;
  }
}

void GWalkDeclaration::add(GASTNode* node, bool isVertex)
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

GWalkDeclaration::WalkElement* GWalkDeclaration::init(GASTNode* node)
{
  auto ptr = (WalkElement*)malloc(sizeof(WalkElement));
  ptr->_next = nullptr;
  ptr->_element = node;
  _size += 1;
  return ptr;
}

GEdgeDeclaration::GEdgeDeclaration(const char* str, GASTNode* node)
  :_from(node),_to(node), _edge(nullptr)
  ,_direction(str)
{

}

GEdgeDeclaration::GEdgeDeclaration(const char* str, GASTNode* from, GASTNode* to)
  :_from(from),_to(to), _edge(nullptr)
  , _direction(str)
{

}

GEdgeDeclaration::GEdgeDeclaration(GASTNode* connection, GASTNode* from, GASTNode* to)
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
    FreeAst(_from);
  }
  FreeAst(_to);
  FreeAst(_edge);
}

GASTNode* GEdgeDeclaration::value() const
{
  if (!_edge) return nullptr;
  if (_edge->_nodetype != EdgeDeclaration) return nullptr;
  GEdgeDeclaration* edge = (GEdgeDeclaration*)_edge->_value;
  return edge ? edge->from() : nullptr;
}
