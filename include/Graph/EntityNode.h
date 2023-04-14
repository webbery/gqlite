#pragma once
#include "base/type.h"
#include "Node.h"
#include "json.hpp"
#include <list>
#include <stdexcept>
#include "Type/GQLType.h"

class GEntityNode: public GNode {
public:
  template<typename T>
  T value(attr_t attribute) {
    // AttributeKind kind = attributeKind(attribute);
  }

  /**
  * If id is 0, it means empty node that have special mean. For example, a super node.
   */
  GEntityNode(node_t id, group_t gid = 0): _id(id), _gid(gid) {
    _status.updated = false;
  }

  std::list<attribute_t> attribute(const std::string& key);

  group_t gid() const { return _gid; }
  const std::multimap<std::string, attribute_t>& attributes() const { return _attributes; }
  node_t id() const { return _id; }

  bool isUpdate() { return _status.updated; }

  void setUpdate(bool update) { _status.updated = update; }
  
  void setProperty(const std::string& key, const attribute_t& value);
private:
  node_t _id;
  group_t _gid;

  std::string _label;
  std::multimap<std::string, attribute_t> _attributes;
};