#pragma once
#include <stdint.h>

// Discribe the graph match pattern.
// It converted by match condition.
struct GraphPattern {
  using node_t = uint32_t;
  using edge_t = uint32_t;

  uint32_t _nsize;  /**< node size */
  node_t* _nodes;   /**< nodes */
  uint32_t _esize;  /**< edge size */
  edge_t* _edges;

  
};