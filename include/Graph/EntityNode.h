#pragma once
#include "base/type.h"
#include "Node.h"
#include "json.hpp"
#include <list>

class GEntityNode: public GNode {
public:
  template<typename T>
  T value(attr_t attribute) {
    // AttributeKind kind = attributeKind(attribute);
  }

  group_t gid() const { return _gid; }
  const nlohmann::json& attributes() const { return _properties; }
  node_t id() const { return _id; }
private:
  node_t _id;
  group_t _gid;
  nlohmann::json _properties;
  edge2_t _eid;
};