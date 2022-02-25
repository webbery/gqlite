#pragma once
#include "base/Serializer.h"
#include "base/Statement.h"

class GEdgeStatment: public GSerializer, public GStatement {
public:
  virtual int Parse(struct gast* ast);
  virtual int Dump();
  virtual std::vector<uint8_t> serialize();
  virtual void deserialize(uint8_t* data, size_t len);
};