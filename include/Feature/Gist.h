#pragma once
#include "Feature.h"

class GBTreeIndex : public GVertexProptertyFeature {
public:
  GBTreeIndex(const char* property);
  ~GBTreeIndex();

  int apply(mdbx::txn_managed& txn, const std::string& id, const std::string& key, const nlohmann::json& value);

private:
};