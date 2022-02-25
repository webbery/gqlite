#include "Feature/InvertIndex.h"
#include "Error.h"
#include <set>

GIndex::GIndex(const char* name)
  :GVertexProptertyFeature(name)
{

}

int GIndex::apply(mdbx::txn_managed& txn, const std::string& id, const std::string& key, const nlohmann::json& value)
{
  if (key == _property) {
    mdbx::map_handle handle;
    switch (value.type())
    {
    case nlohmann::json::value_t::array:
    {
      std::vector<std::string> keys;
      keys = value.get<std::vector<std::string>>();
      handle = create(txn, FeatureType::GTStringFeature);
      updateDisk(txn, id, keys);
    }
      break;
    case nlohmann::json::value_t::string:
    {
      std::vector<std::string> keys;
      handle = create(txn, FeatureType::GTStringFeature);
      std::string reverseKey = value.get<std::string>();
      keys.emplace_back(reverseKey);
      updateDisk(txn, id, keys);
    }
      break;
    case nlohmann::json::value_t::number_integer:
    {
      std::vector<int> keys;
      handle = create(txn, FeatureType::GTIntegerFeature);
      keys.push_back(value.get<int>());
      updateDisk(txn, id, keys);
    }
      break;
    case nlohmann::json::value_t::number_unsigned:
    {
      std::vector<unsigned int> keys;
      handle = create(txn, FeatureType::GTIntegerFeature);
      keys.push_back(value.get<unsigned int>());
      updateDisk(txn, id, keys);
    }
      break;
    default:
      return ECode_BD_Wrong_Type;
    }
  }
  return ECode_Success;
}

GIndex::~GIndex()
{

}

