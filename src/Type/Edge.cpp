#include "Type/Edge.h"
#include "Error.h"
#include "base/ast.h"

enum class GraphEdgeDirection {
  Bidrection,
  To,
};

int GEdgeStatment::Parse(struct gast* ast)
{
  struct gast* arraw = (struct gast*)ast->_value;
  std::string direction = GET_STRING_VALUE(arraw);
  if (direction == "--") {}
  else if (direction == "->") {}
  else if (direction == "<-") {}
  else {
    return ECode_GQL_Edge_Type_Unknow;
  }
  return ECode_Success;
}

int GEdgeStatment::Dump()
{
  return ECode_Success;
}

std::vector<uint8_t> GEdgeStatment::serialize()
{
  std::vector<uint8_t> v;
  return v;
}

void GEdgeStatment::deserialize(uint8_t* data, size_t len)
{

}
