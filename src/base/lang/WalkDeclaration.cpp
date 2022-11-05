#include "base/lang/WalkDeclaration.h"
#include "base/lang/ASTNode.h"

GWalkDeclaration::GWalkDeclaration()
  :_walk(nullptr)
{

}

GWalkDeclaration::~GWalkDeclaration()
{

}

void GWalkDeclaration::add(GASTNode* node, bool isVertex)
{

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
