#include "Feature/Gist.h"
#include "Error.h"
#include "Graph.h"
#include <set>

GBTreeIndex::GBTreeIndex(const char* property)
:GVertexProptertyFeature(property)
{
  _idname = "btree";
}

int GBTreeIndex::apply(mdbx::txn_managed& txn, const std::string& id, const std::string& key, const nlohmann::json& value) {
  if (key == _property) {
    mdbx::map_handle handle;
    std::vector<int32_t> keys;
    switch (value.type())
    {
    case nlohmann::json::value_t::number_integer:
      handle = create(txn, FeatureType::GTIntegerFeature);
      keys.push_back(value.get<int>());
      break;
    case nlohmann::json::value_t::number_unsigned:
      handle = create(txn, FeatureType::GTIntegerFeature);
      keys.push_back(value.get<unsigned int>());
    break;
    default:
      return ECode_BD_Wrong_Type;
    }
    for (int32_t k : keys)
    {
      std::set<VertexID> sIds;
      mdbx::slice data = get(txn, handle, k);
      nlohmann::json j;
      if (data.size()) {
        j = nlohmann::json::from_cbor(data.byte_ptr(), data.byte_ptr() + data.size());
      }
      if (!IS_INVALID_VERTEX(j)) {
        if (j.type() == nlohmann::json::value_t::string) {
          if (j.get<std::string>() == "") continue;
        }
        else {
          sIds = j.get<std::set<VertexID> >();
        }
      }
      sIds.insert(id);
      j = sIds;
      std::vector<uint8_t> bin = nlohmann::json::to_cbor(j);
      data.assign(bin.data(), bin.size());
      put(txn, handle, k, data);
    }
  }
  return 0;
}

GBTreeIndex::~GBTreeIndex() {}

