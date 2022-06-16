#include <map>
#include <functional>
#include <string>
#include <fmt/printf.h>
#include <cassert>
#include "base/lang/ASTNode.h"

#define RETURN_CASE_NODE_TYPE(node_type) case NodeType::node_type: return #node_type
#define RETURN_CASE_PROP_KIND(prop_kind) case PropertyKind::prop_kind: return #prop_kind
#define FREE_NODE(node) case 

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
    RETURN_CASE_NODE_TYPE(Property);
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
    RETURN_CASE_NODE_TYPE(UpsetStatement);
    RETURN_CASE_NODE_TYPE(QueryStatement);
    RETURN_CASE_NODE_TYPE(VertexDeclaration);
    RETURN_CASE_NODE_TYPE(EdgeDeclaration);
    RETURN_CASE_NODE_TYPE(DropStatement);
    RETURN_CASE_NODE_TYPE(RemoveStatement);
  default: return "Unknow Node Type: " + std::to_string((int)nt);
  }
}

std::string PropertyKind2String(PropertyKind kind) {
  switch (kind) {
    RETURN_CASE_PROP_KIND(Binary);
    RETURN_CASE_PROP_KIND(Number);
    RETURN_CASE_PROP_KIND(String);
  default: return "Unknow Property Kind: " + std::to_string((int)kind);
  }
}

void printLine(const std::string& format, const std::string& str, int level) {
  for (int lvl = 0; lvl < level; ++lvl) {
    fmt::printf("  ");
  }
  fmt::print(format, str);
}

std::function<void(GASTNode* , int )> getPrintFunc(NodeType nt) {
  static std::map<NodeType, std::function<void(GASTNode* root, int level)> > mPrinter({
    {NodeType::Literal,
      [](GASTNode* root, int level) {
        GTypeTraits<NodeType::Literal>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::Literal>::type*>(root->_value);
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
        printLine("|- raw: {}\n", ptr->raw(), level);
        printLine("|- kind: {}\n", PropertyKind2String(ptr->kind()), level);
      }
    },
    {NodeType::GQLExpression,
      [](GASTNode* root, int level) {
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
      }
    },
    {NodeType::CreationStatement,
      [](GASTNode* root, int level) {
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
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
    },
    {NodeType::UpsetStatement,
      [](GASTNode* root, int level) {
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
        GTypeTraits<NodeType::UpsetStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::UpsetStatement>::type*>(root->_value);
        printLine("|- name: {}\n", ptr->name(), level);
      }
    },
    {NodeType::ArrayExpression,
      [](GASTNode* root, int level) {
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
        GASTNode* ptr = root;
        size_t cnt = 1;
        while (ptr) {
          printLine("|- #{}\n", std::to_string(cnt++), level + 1);
          DumpAst((GASTNode*)ptr->_value, level + 2);
          // next 
          ptr = ptr->_children;
        }
      }
    },
    {NodeType::ObjectExpression,
      [](GASTNode* root, int level) {
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
        DumpAst((GASTNode*)root->_value, level + 1);
      }
    },
    {NodeType::Property,
      [](GASTNode* root, int level) {
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
        GTypeTraits<NodeType::Property>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::Property>::type*>(root->_value);
        printLine("|- key: {}\n", ptr->key(), level);
        printLine("|- value:\n", "", level);
        DumpAst(ptr->value(), level + 1);
      }
    },
    {NodeType::QueryStatement,
      [](GASTNode* root, int level) {
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
        GTypeTraits<NodeType::QueryStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::QueryStatement>::type*>(root->_value);
        printLine("|- query:\n", "", level);
        DumpAst(ptr->query(), level + 1);
        printLine("|- graph:\n", "", level);
        DumpAst(ptr->graph(), level + 1);
        if (ptr->where()) {
          printLine("|- where:\n", "", level);
          DumpAst(ptr->where(), level + 1);
        }
      }
    },
    {NodeType::BinaryExpression,
      [](GASTNode* root, int level) {
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
        GTypeTraits<NodeType::BinaryExpression>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::BinaryExpression>::type*>(root->_value);
        printLine("|- name: {}\n", ptr->key(), level);
        printLine("|- value:\n", "", level);
        DumpAst(ptr->value(), level + 1);
      }
    },
    {NodeType::VertexDeclaration,
      [](GASTNode* root, int level) {
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
        GTypeTraits<NodeType::VertexDeclaration>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::VertexDeclaration>::type*>(root->_value);
        DumpAst(ptr->vertex(), level + 1);
      }
    },
    {NodeType::EdgeDeclaration,
      [](GASTNode* root, int level) {
        std::string nodeType = NodeType2String(root->_nodetype);
        printLine("|- type: {}\n", nodeType, level);
        GASTNode* ptr = root;
        size_t cnt = 1;
        while (ptr) {
          GTypeTraits<NodeType::EdgeDeclaration>::type* edge = reinterpret_cast<GTypeTraits<NodeType::EdgeDeclaration>::type*>(ptr->_value);
          printLine("|- #{}\n", std::to_string(cnt++), level);
          printLine("|- #from\n", "", level + 1);
          DumpAst(edge->from(), level + 2);
          printLine("|- #\n", "", level + 1);
          DumpAst(edge->link(), level + 2);
          printLine("|- #to\n", "", level + 1);
          DumpAst(edge->to(), level + 2);
          ptr = ptr->_children;
        }
      }
    },
  });
  assert(mPrinter.count(nt) > 0);
  return mPrinter[nt];
}

void DumpAst(GASTNode* root, int level /* = 0 */) {
  if (root == nullptr) return;
  getPrintFunc(root->_nodetype)(root, level);
  for (size_t idx = 0; idx < root->_size; ++idx) {
    DumpAst(root->_children + idx, level + 1);
  }
}

std::string GetString(GASTNode* node) {
  GLiteral* str = (GLiteral*)node->_value;
  return str->raw();
}