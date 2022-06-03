#include "Type/VertexStmt.h"
#include "Error.h"
#include "gqlite.h"

std::vector<uint8_t> GVertexStmt::serialize()
{
  std::vector<uint8_t> v;
  if (_json.empty()) return v;
  v = nlohmann::json::to_cbor(_json);
  uint8_t extra = 0;
  if (_binary) {
    extra |= EXTERN_BINARY_BIT;
  }
  v.emplace_back(extra);
  return v;
}

void GVertexStmt::deserialize(uint8_t* data, size_t len)
{

}

int GVertexStmt::Parse(struct gast* ast)
{
  //struct gast* left = ast->_left;
  //if (left) _id = GET_STRING_VALUE(left->_right);
  //struct gast* right = ast->_right;
  //if (right) {
  //  ArrayVisitor av;
  //  GET_VALUE((gast*)right->_value, _binary, av);
  //  _json[_id] = av.value();
  //  //_json = GET_ARRAY_VALUE((gast*)right->_value, _binary);
  //}
  //else {
  //  _json[this->_id] = "";
  //}
  return ECode_Success;
}

int GVertexStmt::Dump()
{
  return ECode_Success;
}

nlohmann::json GVertexStmt::value(const std::string& key)
{
  if (!_json.contains(key)) return nlohmann::json();
  return _json[key];
}
