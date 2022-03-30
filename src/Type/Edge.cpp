#include "Type/Edge.h"
#include "gqlite.h"
#include "base/ast.h"

edge_id::edge_id(const std::string& from, const std::string& to, GraphEdgeDirection direction)
:_direction(direction)
{
  switch (direction)
  {
  case GraphEdgeDirection::Bidrection:
    _from = from;
    _to = to;
    break;
  case GraphEdgeDirection::To:
    break;
  default:
    break;
  }
}

std::string edge_id::from() const
{
  return _from;
}

std::string edge_id::to() const
{
  return _to;
}

bool edge_id::isDirection() const
{
  return _direction == GraphEdgeDirection::To;
}

bool edge_id::operator!=(const edge_id& other) const
{
  return !equal(other);
}

std::string edge_id::str()const
{
  switch (_direction)
  {
  case GraphEdgeDirection::Bidrection:
    return _from + "--" + _to;
  case GraphEdgeDirection::To:
    return _from + "->" + _to;
  default:
    break;
  }
  return "";
}

bool edge_id::equal(const edge_id& other) const
{
  if (_direction != other._direction) return false;
  switch (_direction)
  {
  case GraphEdgeDirection::Bidrection:
    if (_from == other._from && _to == other._to) return true;
    if (_from == other._to && _to == other._from) return true;
    break;
  case GraphEdgeDirection::To:
    if (_from == other._from && _to == other._to) return true;
    break;
  default:
    break;
  }
  return false;
}

bool edge_id::lt(const edge_id& other) const
{
  return std::string(*this) < (std::string)other;
}

bool edge_id::operator==(const edge_id& other) const
{
  return equal(other);
}

bool edge_id::operator!=(const edge_id& other)
{
  return !equal(other);
}

bool edge_id::operator<(const edge_id& other)
{
  return lt(other);
}

bool edge_id::operator<(const edge_id& other)const
{
  return lt(other);
}

edge_id::edge_id(const std::string& eid)
{
}

bool edge_id::operator == (const edge_id& other)
{
  return equal(other);
}

edge_id::operator std::string()
{
  return str();
}

edge_id::operator std::string() const
{
  return str();
}

GEdge::GEdge(const std::string& from, const std::string& to, GraphEdgeDirection direction)
:_id(from, to, direction)
{

}

std::string GEdge::id()
{
  return _id;
}

std::string GEdge::from() const
{
  return _id.from();
}

std::string GEdge::to() const
{
  return _id.to();
}

