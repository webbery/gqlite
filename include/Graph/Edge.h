#pragma once
#include <stdint.h>

enum class EdgeKind: uint8_t {
  Entity,
  Attribute,
  Literal,
  MAX
};

class GEdge {
public:
  virtual ~GEdge() {}
  struct alignas(8) Status {
    bool updated: 1;      // is this node updated
    bool hold: 1;
    EdgeKind kind: 3;
  };
};