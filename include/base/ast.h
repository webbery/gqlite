#pragma once
#include "type.h"
#include "json.hpp"

struct gast {
  enum NodeType _nodetype;
  void* _value;
  struct gast* _left;
  struct gast* _right;
};

struct gast* newast(enum NodeType type, void* value, struct gast* left, struct gast* right);
void freeast(struct gast* root);
void dumpast(const char* filename, struct gast* root);
struct gast* loadast(const char* filename);

struct gql_node* as_array(struct gast* root);

class ASTVisitor;
void traverse(struct gast*, ASTVisitor* visitor);

nlohmann::json GET_ARRAY_VALUE(struct gast* ast, bool& hasBinary);
nlohmann::json GET_PROPERTY_VALUE(const std::string& key, struct gast* ast, bool& hasBinary);
double GET_NUMBER_VALUE(struct gast* ast);
std::string GET_STRING_VALUE(struct gast* ast);
