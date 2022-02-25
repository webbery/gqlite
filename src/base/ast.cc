#include "base/ast.h"
#include "base/list.h"
#include <memory>
#include "base/visitor.h"
#include <fstream>

struct gast* newast(enum NodeType type, void* value, struct gast* left, struct gast* right) {
  struct gast* ast = (struct gast*)malloc(sizeof(struct gast));
  ast->_value = value;
  ast->_nodetype = type;
  ast->_left = left;
  ast->_right = right;
  return ast;
}

void freeast(struct gast* root) {
  if (!root) return;
  if (!root->_value) return;
  freeast(root->_left);
  freeast(root->_right);
  free(root->_value);
}

void dumpast(const char* filename, struct gast* root) {
  // dump ast to json
}

struct gast* loadast(const char* filename) {
  return nullptr;
}

struct gql_node* as_array(struct gast* root) {
  if (!root) return 0;
  if (root->_nodetype != ArrayExpression) return 0;
  return (struct gql_node*)root->_value;
}

void traverse(struct gast* value, ASTVisitor* visitor)
{
  switch (value->_nodetype) {
  case NodeType::ArrayExpression:
  {
    /* array
    *     |- property
    *   |- element
    *   ...
    */
    gql_node* cur = (gql_node*)value->_value;
    while (cur)
    {
      visitor->visit(value->_nodetype, cur->_value);
      cur = cur->_next;
    }
  }
  break;
  case NodeType::ObjectExpression:
  {
    struct gast* cur = (struct gast*)value->_value;
    visitor->visit(value->_nodetype, cur);
    traverse(cur, visitor);
  }
  break;
  case NodeType::Property:
  {
    struct gast* left = (struct gast*)value->_left;
    char* p = (char*)left->_value;
    size_t len = strlen(p) + 1;
    visitor->visit(NodeType::String, p);
    std::string key(p , len);
    struct gast* right = (struct gast*)value->_right;
    traverse(right, visitor);
  }
    break;
  case NodeType::String:
  {
    //struct gast* prop = (struct gast*)cur->_value;
    //char* p = (char*)prop->_left->_value;
    //size_t len = strlen(p) + 1;
  }
    break;
  case NodeType::Edge:
  case NodeType::Vertex:
  {
    visitor->visit(value->_nodetype, value);
  }
    break;
  default:
    break;
  }
}


std::string GET_STRING_VALUE(struct gast* ast) {
  char* pid = (char*)ast->_value;
  size_t len = strlen(pid);
  return std::string(pid, len);
}

double GET_NUMBER_VALUE(struct gast* ast) {
  double* d = (double*)ast->_value;
  return *d;
}

nlohmann::json GET_PROPERTY_VALUE(const std::string& key, struct gast* ast, bool& hasBinary) {
  nlohmann::json result;
  if (!ast) return result;
  switch (ast->_nodetype)
  {
  case NodeType::Property:
  {
    std::string k = GET_STRING_VALUE(ast->_left);
    result[key] = GET_PROPERTY_VALUE(k, ast->_right, hasBinary);
  }
  case NodeType::String:
    result[key] = GET_STRING_VALUE(ast);
    break;
  case NodeType::Number:
    result[key] = GET_NUMBER_VALUE(ast);
    break;
  case NodeType::Binary:
    hasBinary = true;
    break;
  case NodeType::ArrayExpression:
    result[key] = GET_ARRAY_VALUE(ast, hasBinary);
    break;
  default:
    break;
  }
  return result;
}

nlohmann::json GET_ARRAY_VALUE(struct gast* ast, bool& hasBinary) {
  nlohmann::json result;
  gql_node* node = (gql_node*)ast->_value;
  while (node)
  {
    gast* item = (gast*)node->_value;
    switch (item->_nodetype)
    {
    case NodeType::Property:
    {
      std::string key = GET_STRING_VALUE(item->_left);
      nlohmann::json value = GET_PROPERTY_VALUE(key, item->_right, hasBinary);
      result.push_back(value);
    }
    break;
    case NodeType::String:
      result.push_back(GET_STRING_VALUE(item));
      break;
    case NodeType::Number:
      result.push_back(GET_NUMBER_VALUE(item));
      break;
    case NodeType::Binary:
      hasBinary = true;
      break;
    case NodeType::ArrayExpression:
      result.push_back(GET_ARRAY_VALUE(item, hasBinary));
      break;
    default:
      break;
    }
    node = node->_next;
  }
  return result;
}