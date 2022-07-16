#pragma once
#include <vector>
#include <functional>

struct EntityNode;
struct AttributeNode;
struct EntityEdge {
  EntityNode* _start;
  EntityNode* _end;
  char* _label;
};

struct EntityNode {
  char* _label;
  EntityEdge* _edges;
  size_t _esize;        /**< size of edges */
  AttributeNode* _attrs;
};

struct AttributeNode {};

// describe the graph match pattern.
// It come from query condition.
struct GraphPattern {
  using node_t = uint32_t;

  std::vector<EntityNode*> _nodes;
  std::vector<EntityEdge*> _edges;

  /**
   * @brief a predicate apply to node label
   * 
   */
  std::function<bool (char*)> _opl;
  /**
   * @brief a predicate apply to node's attribute
   * 
   */
  std::function<bool (node_t)> _opa;
};