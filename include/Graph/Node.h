#pragma once
#include <stdint.h>

enum class NodeKind: uint8_t {
  Entity,
  Attribute,
  Literal,
  MAX
};

class GNode {
public:
  virtual ~GNode() {}

  struct alignas(8) Status {
    bool visit: 1;        // visit state of current plan. For example, 1 is visited in last plan, 0 is current plan.
    bool updated: 1;      // is this node updated
    NodeKind kind: 4;
  };

  Status _status;
};