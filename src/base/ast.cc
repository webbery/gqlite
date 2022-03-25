#include "base/ast.h"
#include "base/list.h"
#include <memory>
#include "base/visitor.h"
#include <fmt/format.h>

namespace {
  void print_node(gast* node, int level) {
    // fmt::print("\t");
    switch(node->_nodetype) {
    case NodeType::Number:
      fmt::print("{1}: {0:\t^{1}}Type: Number\n", "",level);
      break;
    case NodeType::String:
      fmt::print("{1}: {0:\t^{1}}Type: String, Value: {2}\n", "", level, (char*)node->_value);
      break;
    case NodeType::Property:
      fmt::print("{1}: {0:\t^{1}}Type: Property\n", "", level);
      break;
    case NodeType::ArrayExpression:
      fmt::print("{1}: {0:\t^{1}}Type: ArrayExpression\n", "", level);
      break;
    case NodeType::UpsetStatement:
      fmt::print("{1}: {0:\t^{1}}Type: UpsetStatement\n", "", level);
      {
        gql_node* cur = (gql_node*)node->_value;
        level += 1;
        while(cur) {
          dumpast((gast*)cur->_value, level);
          cur = cur->_next;
        }
      }
      break;
    case NodeType::Vertex:
      fmt::print("{1}: {0:\t^{1}}Type: Vertex\n", "", level);
      break;
    case NodeType::Binary:
      break;
    default:
      fmt::print("{1}: {0:\t^{1}}Type: Undefined[{2}]\n", "", level, node->_nodetype);
      break;
    }
  }
}

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

void recusive_ast(struct gast* ast) {
  if (!ast) return;
}

void dumpast(struct gast* root, int level) {
  if (!root) {
    return;
  }
  print_node(root, level);
  level += 1;
  dumpast(root->_left, level);
  dumpast(root->_right, level);
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
  // printf("\t%s\n", pid);
  return std::string(pid, len);
}

double GET_NUMBER_VALUE(struct gast* ast) {
  double* d = (double*)ast->_value;
  return *d;
}

int32_t GET_INT_VALUE(struct gast* ast)
{
  int32_t* d = (int32_t*)ast->_value;
  return *d;
}

void ObjectVisitor::visit(Acceptor<NodeType::Property>& acceptor)
{
  _value.update(acceptor.value(), true);
}

void ObjectVisitor::visit(Acceptor<NodeType::Number>& acceptor)
{
  printf("ObjectVisitor Number: %f\n", acceptor.value());
}

void ObjectVisitor::visit(Acceptor<NodeType::ArrayExpression>& acceptor)
{
}

void ObjectVisitor::visit(Acceptor<NodeType::String>& acceptor)
{
  printf("ObjectVisitor String: %s\n", acceptor.value().c_str());
}

void ObjectVisitor::visit(Acceptor<NodeType::ObjectExpression>& acceptor)
{
  printf("ObjectVisitor ObjectExpression: %s\n", acceptor.value().dump().c_str());
}

void ObjectVisitor::visit(Acceptor<NodeType::Integer>& acceptor)
{
  printf("ObjectVisitor Number: %d\n", acceptor.value());
}
