#pragma once
#include "base/Serializer.h"
#include "base/Statement.h"
#include "json.hpp"

class GVertexStatment: public GSerializer, public GStatement {
public:
  virtual std::vector<uint8_t> serialize();
  virtual void deserialize(uint8_t* data, size_t len);
  virtual int Parse(struct gast* ast);
  virtual int Dump();

  std::string id() { return _id; }
  nlohmann::json value(const std::string& key);

private:
  std::string _id;
  bool _binary = false;
  nlohmann::json _json;
};