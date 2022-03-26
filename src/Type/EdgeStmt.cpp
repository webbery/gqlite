#include "Type/EdgeStmt.h"
#include "base/ast.h"
#include "gqlite.h"

GEdgeStatment::GEdgeStatment()
  :GEdge("", "", GraphEdgeDirection::Bidrection) {}

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
