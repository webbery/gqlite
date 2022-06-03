#include "base/lang/ASTNode.h"
#include "base/lang/GQLExpression.h"
#include "base/lang/Literal.h"
#include "base/lang/CreateStmt.h"
#include <map>
#include <functional>
#include <string>
#include <fmt/printf.h>

#define RETURN_CASE_NODE_TYPE(node_type) case NodeType::node_type: return #node_type
#define FREE_NODE(node) case 

template <NodeType T> struct GTypeTraits {};

template <> struct GTypeTraits<NodeType::GQLExpression> {
  typedef GGQLExpression type;
};
template <> struct GTypeTraits<NodeType::Literal> {
  typedef GLiteral type;
};

template <> struct GTypeTraits<NodeType::CreationStatement> {
  typedef GCreateStmt type;
};

GASTNode* ListJoin(GASTNode* first, GASTNode* second) {
  if (!first) return second;
  if (!second) return first;
  GASTNode* cur = first;
  while (cur->_children) cur = cur->_children;
  cur->_children = second;
  return first;
}

void FreeNodeType(GASTNode* node) {
  std::map<NodeType, std::function<void (void*, size_t)> > release({
    {NodeType::Literal, [](void* ptr, size_t len) {}}
  });
}

GASTNode* NewAst(enum NodeType type, void* value, GASTNode* children, size_t size) {
  GASTNode* ast = (struct GASTNode*)malloc(sizeof(struct GASTNode));
  ast->_value = value;
  ast->_nodetype = type;
  ast->_children = children;
  ast->_size = size;
  return ast;
}

void FreeAst(GASTNode* root) {
  if (!root) return;
  if (!root->_value) return;
  for (size_t idx = 0; idx < root->_size; ++idx) {
    FreeAst(root->_children + idx);
  }
  FreeNodeType(root);
}

std::string NodeType2String(NodeType nt) {
  switch (nt) {
    RETURN_CASE_NODE_TYPE(Literal);
    RETURN_CASE_NODE_TYPE(Identifier);
    RETURN_CASE_NODE_TYPE(Program);
    RETURN_CASE_NODE_TYPE(FunctionDeclaration);
    RETURN_CASE_NODE_TYPE(VariableDeclaration);
    RETURN_CASE_NODE_TYPE(VariableDeclarator);
    RETURN_CASE_NODE_TYPE(BlockStatement);
    RETURN_CASE_NODE_TYPE(ReturnStatement);
    RETURN_CASE_NODE_TYPE(BinaryExpression);
    RETURN_CASE_NODE_TYPE(ArrayExpression);
    RETURN_CASE_NODE_TYPE(ObjectExpression);
    RETURN_CASE_NODE_TYPE(GQLExpression);
    RETURN_CASE_NODE_TYPE(CallExpression);
    RETURN_CASE_NODE_TYPE(CreationStatement);
  default: return "Unknow: " + std::to_string((int)nt);
  }
}

void printLine(const std::string& format, const std::string& str, int level) {
  for (int lvl = 0; lvl < level; ++lvl) {
    fmt::printf("  ");
  }
  fmt::print(format, str);
}

void DumpAst(GASTNode* root, int level /* = 0 */) {
  if (root == nullptr) return;
  std::string nodeType = NodeType2String(root->_nodetype);
  printLine("|- type: {}\n", nodeType, level);
  switch (root->_nodetype) {
    case NodeType::Literal:
    {
      GTypeTraits<NodeType::Literal>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::Literal>::type*>(root->_value);
      printLine("|- raw: {}\n", ptr->raw(), level);
    }
      break;
    case NodeType::GQLExpression:
    {
      // GTypeTraits<NodeType::GQLExpression>::type* ptr = (GTypeTraits<NodeType::GQLExpression>::type*)(root->_value);
    }
      break;
    case NodeType::CreationStatement:
    {
      GTypeTraits<NodeType::CreationStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::CreationStatement>::type*>(root->_value);
      printLine("|- name: {}\n", ptr->name(), level);
      GASTNode* list = ptr->indexes();
      size_t cnt = 1;
      while (list) {
        printLine("|- index#{}\n", std::to_string(cnt++), level);
        GASTNode* node = (GASTNode*)list->_value;
        DumpAst(node, level + 1);
        list = list->_children;
      }
    }
      break;
    default:
      break;
  }
  for (size_t idx = 0; idx < root->_size; ++idx) {
    DumpAst(root->_children + idx, level + 1);
  }
}