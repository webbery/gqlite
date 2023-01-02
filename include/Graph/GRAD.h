#pragma once
#include <vector>
#include <functional>
#include <string>
#include "base/Variant.h"
#include "Type/GQLType.h"

struct EntityNode;
struct AttributeNode;

struct EntityEdge {
  EntityNode* _start;
  EntityNode* _end;
  char* _label;
  /**
   * Super Edge when _extend is true
   */
  bool _extend;
  bool _direction;
};

using gkey_t = Variant<std::string, uint64_t>;
using attribute_t = Variant<std::string, double, gql::GDatetime, gql::vector_double, gql::GBinary, int, long, uint64_t,
  gql::vector_uint8
>;
using predicate_t = Variant<std::function<bool(const gkey_t&)>, std::function<bool(const attribute_t&)>>;
using attr_node_t = std::string;

struct EntityNode {
  std::string _label;
  EntityEdge* _edges;
  size_t _esize;        /**< size of edges */
  std::vector<attr_node_t> _attrs;
};

struct AttributeNode {
  std::string _name;
};

enum class LogicalPredicate {
  And = 0,
  Or = 1,
  Max
};

/**
 * @brief Describe the graph match pattern.
 *        It come from query condition. For example, if query some vertexes by attributes,
 *        _nodes must have a node object, it's attributes are same as query attributes.
 *        It means that predict attributes are assign to node's `_attr`.
 *        If `_attr` is empty, it means that all of data should be scan.
 *        If query is edge, _edges shouldn't empty.
 */ 
struct GraphPattern {
  using node_t = uint32_t;

  std::vector<EntityNode*> _nodes;
  std::vector<EntityEdge*> _edges;

  /**
   * Predicates index is response to _nodes's attributes index.
   * So they have same size.
   */
  std::vector<predicate_t> _node_predicates;
};

struct QueryCondition {
  GraphPattern _patterns[(long)LogicalPredicate::Max];
};