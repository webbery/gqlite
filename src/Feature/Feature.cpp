#include "Feature/Feature.h"

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

