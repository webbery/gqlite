#pragma once
#include "Node.h"
#include "base/system/EventEmitter.h"

class GVirtualNetwork;
class GEntityNode: public GNode {
public:
  using group_t = uint8_t;
  GEntityNode(GEventEmitter& event);

  template<typename T>
  T value(attr_t attribute) {
    // AttributeKind kind = attributeKind(attribute);
  }
private:
  node_t _id;
  group_t _gid;
};