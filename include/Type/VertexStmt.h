#pragma once
#include "Vertex.h"

class GVertexStmt : public GSerializer, public GStatement, public GVertex {
public:
  // virtual ~GVertexStmt() { printf("~GVertexStmt\n"); }
  virtual std::vector<uint8_t> serialize();
  virtual void deserialize(uint8_t* data, size_t len);
  virtual int Parse(struct gast* ast);
  virtual int Dump();

  nlohmann::json value(const std::string& key);
  bool setBinaryFlag(bool flag) { _binary = flag; return true; }
  bool hasBinary() { return _binary; }

private:
  bool _binary = false;
};
