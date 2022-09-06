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
};

using gkey_t = Variant<std::string, uint64_t>;
using attribute_t = Variant<std::string, double, gql::GDatetime, gql::vector_double, gql::GBinary, int>;
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

// describe the graph match pattern.
// It come from query condition.
// this GRAD is data structure, which different from gql_mal
struct GraphPattern {
  using node_t = uint32_t;

  std::vector<EntityNode*> _nodes;
  std::vector<EntityEdge*> _edges;

  std::vector< predicate_t > _node_predicates[(long)LogicalPredicate::Max];
  
};