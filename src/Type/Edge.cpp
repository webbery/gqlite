#include "Type/Edge.h"
#include "Error.h"
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

edge_id::edge_id(const std::string& eid)
{

}

bool edge_id::operator == (const edge_id& other)
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

edge_id::operator std::string()
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

int GEdgeStatment::Parse(struct gast* ast)
{
  struct gast* arraw = (struct gast*)ast->_value;
  gast* property = (struct gast*)ast->_left;
  std::string left = GET_STRING_VALUE((struct gast*)property->_right);
  property = (struct gast*)ast->_right;
  std::string right = GET_STRING_VALUE((struct gast*)property->_right);
  std::string direction = GET_STRING_VALUE(arraw);
  if (direction == "--") {
    edge_id eid(left, right);
    _id = (std::string)eid;
  }
  else if (direction == "->") {
    edge_id eid(left, right, GraphEdgeDirection::To);
    _id = (std::string)eid;
  }
  else if (direction == "<-") {
    edge_id eid(right, left, GraphEdgeDirection::To);
    _id = (std::string)eid;
  }
  else {
    return ECode_GQL_Edge_Type_Unknow;
  }
  _json[_id] = "";
  return ECode_Success;
}

int GEdgeStatment::Dump()
{
  return ECode_Success;
}

std::vector<uint8_t> GEdgeStatment::serialize()
{
  std::vector<uint8_t> v;
  if (_json.empty()) return v;
  v = nlohmann::json::to_cbor(_json);
  return v;
}

void GEdgeStatment::deserialize(uint8_t* data, size_t len)
{

}

std::string GEdgeStatment::id()
{
  return _id;
}
