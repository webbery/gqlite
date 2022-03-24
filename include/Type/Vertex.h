#pragma once
#include "base/Serializer.h"
#include "base/Statement.h"
#include "json.hpp"

typedef std::string GLiteralVertex;

class GAttributeVertex: public GSerializer {
public:
private:
  std::vector<GLiteralVertex> _names;
};

class GVertex {
public:
  virtual ~GVertex() {}

  std::string id() { return _id; }
  const nlohmann::json& property() const { return _json; }
  nlohmann::json& property() { return _json; }

  int set(const std::string& id, const nlohmann::json& prop) {
    _id = id;
    _json = prop;
    return 0;
  }

protected:
  std::string _id;
  nlohmann::json _json;
};

class GVertexStmt: public GSerializer, public GStatement, public GVertex {
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
