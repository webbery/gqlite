#pragma once
#include <stdint.h>
#include <atomic>

enum class NodeKind: uint8_t {
  Entity,
  Attribute,
  Literal,
  MAX
};

struct alignas(8) NodeStatus {
  bool visit : 1;        // visit state of current plan. For example, 1 is visited in last plan, 0 is current plan.
  bool hold : 1;
  bool del : 1;
  NodeKind kind : 2;
  uint8_t layer : 3;     // hnsw layer level
  uint8_t color : 3;     // color for partition
  bool updated : 1;      // is this node updated
  uint8_t reserve: 4;
};

class GNode {
protected:
  virtual ~GNode() {}

  NodeStatus _status;
};