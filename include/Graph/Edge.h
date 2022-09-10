#pragma once
#include <stdint.h>

enum class EdgeKind: uint8_t {
  Entity,
  Attribute,
  Literal,
  MAX
};

struct alignas(8) EdgeStatus {
  bool updated : 1;      // is this node updated
  bool hold : 1;
  //EdgeKind kind : 2;
  bool del : 1;     // this node is marked as delete or not
  /**
   * High bit means direction. 1 - is direction, 0 - is bidirection
   * Low bit means direction is out or in. 0 - out, 1 - in
   * Bidirection must be 00. Other wise 01 mean special case to be check.
   * Direction can be 10/11.
   */
  uint8_t direction : 2;
  uint8_t layer : 2;     // hnsw layer level
};

class GEdge {
public:
  virtual ~GEdge() {}
};