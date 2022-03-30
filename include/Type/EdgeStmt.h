#pragma once
#include "base/Statement.h"
#include "base/Serializer.h"
#include "Edge.h"

class GEdgeStatment : public GSerializer, public GStatement, public GEdge {
public:
  GEdgeStatment();
  virtual int Parse(struct gast* ast);
  virtual int Dump();
  virtual std::vector<uint8_t> serialize();
  virtual void deserialize(uint8_t* data, size_t len);
};
