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
};

using key_t = Variant<std::string, uint64_t>;
using attribute_t = Variant<std::string, double, gql::GDatetime>;
using predicate_t = Variant<std::function<bool(const key_t&)>, std::function<bool(const attribute_t&)>>;
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
struct GraphPattern {
  using node_t = uint32_t;

  std::vector<EntityNode*> _nodes;
  std::vector<EntityEdge*> _edges;

  std::vector< predicate_t > _node_predicates[(long)LogicalPredicate::Max];
  
};