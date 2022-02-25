#pragma once
#include <mdbx.h++>
#include "./Feature.h"
#include "Graph.h"

class GIndex: public GVertexProptertyFeature {
public:
  GIndex(const char* name);

  virtual int apply(mdbx::txn_managed& txn, const std::string& id, const std::string& key, const nlohmann::json& value);

  virtual ~GIndex();

private:
  template<typename T>
  void updateDisk(mdbx::txn_managed& txn, const std::string& id, std::vector<T>& keys) {
    for (T& k : keys)
    {
      std::set<VertexID> sIds;
      mdbx::slice data = get(txn, _handle, k);
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
      put(txn, _handle, k, data);
    }
  }
};