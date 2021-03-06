#pragma once
#include <stdint.h>
#include <atomic>

enum class NodeKind: uint8_t {
  Entity,
  Attribute,
  Literal,
  MAX
};

class GNode {
public:
  using attr_t = uint8_t;
  using node_t = uint32_t;

  virtual ~GNode() {}

  struct alignas(8) Status {
    bool visit: 1;        // visit state of current plan. For example, 1 is visited in last plan, 0 is current plan.
    bool updated: 1;      // is this node updated
    bool hold: 1;
    NodeKind kind: 3;
    uint8_t layer: 2;     // hnsw layer level
  };

  std::atomic<Status> _status;
};