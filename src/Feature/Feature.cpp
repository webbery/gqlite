#include "Feature/Feature.h"
#include "gqlite.h"

GVertexProptertyFeature::GVertexProptertyFeature(const char* property)
  :_property(property)
  ,_type(FeatureType::GTUndefined)
{

}

GVertexProptertyFeature::~GVertexProptertyFeature()
{
  if (_handle.dbi) {

  }
}

int GVertexProptertyFeature::get_cursor(mdbx::txn_managed& txn, const std::any& pos, NodeType nt, mdbx::cursor_managed& cursor)
{
  switch (nt)
  {
  case NodeType::Number:
    // index must be integer not double
    anchor_cursor(txn, std::any_cast<uint64_t>(pos), cursor);
    return ECode_Success;
  case NodeType::String:
    break;
  default:
    break;
  }
  return ECode_Fail;
}

mdbx::map_handle& GVertexProptertyFeature::create(mdbx::txn_managed& txn, FeatureType type) {
  if (!_handle.dbi && type == FeatureType::GTStringFeature) {
    _handle = txn.create_map(_property, mdbx::key_mode::usual, mdbx::value_mode::single);
  }
  else if (!_handle.dbi && type == FeatureType::GTIntegerFeature) {
    _handle = txn.create_map(_property, mdbx::key_mode::ordinal, mdbx::value_mode::single);
  }
  return _handle;
}

bool GVertexProptertyFeature::drop(mdbx::txn_managed& txn) {
  if (!_handle.dbi) return false;
  txn.drop_map(_handle);
  return true;
}

GIDFeature::GIDFeature(mdbx::map_handle vertex)
  :GVertexProptertyFeature("id")
{
  _handle = vertex;
}

int GIDFeature::apply(mdbx::txn_managed& txn, const std::string& id, const std::string& key, const nlohmann::json& value)
{
  return ECode_Success;
}

int GIDFeature::get_cursor(mdbx::txn_managed& txn, const std::any& pos, NodeType nt, mdbx::cursor_managed& cursor)
{
  std::string s = std::any_cast<std::string>(pos);
  std::addressof(s);
  return move_cursor(txn, _handle, s, cursor);
}
