#pragma once
#include "base/type.h"
#include "Node.h"
#include "json.hpp"
#include <list>
#include <stdexcept>

class GEntityNode: public GNode {
public:
  template<typename T>
  T value(attr_t attribute) {
    // AttributeKind kind = attributeKind(attribute);
  }

  GEntityNode(node_t id, group_t gid): _id(id), _gid(gid) {
    _status.updated = false;
  }

  void setProperties(const nlohmann::json& properties) { _properties = properties; }

  group_t gid() const { return _gid; }
  const nlohmann::json& attributes() const { return _properties; }
  node_t id() const { return _id; }

  bool isUpdate() { return _status.updated; }

  void setUpdate(bool update) { _status.updated = update; }
  
private:
  node_t _id;
  group_t _gid;
  nlohmann::json _properties;

  edge2_t _eid;
};