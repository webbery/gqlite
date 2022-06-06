#include "base/lang/lang.h"
#include <map>
#include <functional>
#include <string>
#include <fmt/printf.h>

#define RETURN_CASE_NODE_TYPE(node_type) case NodeType::node_type: return #node_type
#define RETURN_CASE_PROP_KIND(prop_kind) case PropertyKind::prop_kind: return #prop_kind
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

template <> struct GTypeTraits<NodeType::UpsetStatement> {
  typedef GUpsetStmt type;
};

template <> struct GTypeTraits<NodeType::ArrayExpression> {
  typedef GASTNode type;
};

template <> struct GTypeTraits<NodeType::Property> {
  typedef GProperty type;
};

template <> struct GTypeTraits<NodeType::BinaryExpression> {
  typedef GProperty type;
};

template <> struct GTypeTraits<NodeType::QueryStatement> {
  typedef GQueryStmt type;
};

template <> struct GTypeTraits<NodeType::VertexDeclaration> {
  typedef GVertexDeclaration type;
};

template <> struct GTypeTraits<NodeType::EdgeDeclaration> {
  typedef GEdgeDeclaration type;
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

void DumpAst(GASTNode* root, int level /* = 0 */) {
  if (root == nullptr) return;
  std::string nodeType = NodeType2String(root->_nodetype);
  printLine("|- type: {}\n", nodeType, level);
  switch (root->_nodetype) {
    case NodeType::Literal:
    {
      GTypeTraits<NodeType::Literal>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::Literal>::type*>(root->_value);
      printLine("|- raw: {}\n", ptr->raw(), level);
      printLine("|- kind: {}\n", PropertyKind2String(ptr->kind()), level);
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
    case NodeType::UpsetStatement:
    {
      GTypeTraits<NodeType::UpsetStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::UpsetStatement>::type*>(root->_value);
      printLine("|- name: {}\n", ptr->name(), level);
    }
      break;
    case NodeType::ArrayExpression:
    {
      GASTNode* ptr = root;
      size_t cnt = 1;
      while (ptr) {
        printLine("|- #{}\n", std::to_string(cnt++), level + 1);
        DumpAst((GASTNode*)ptr->_value, level + 2);
        // next 
        ptr = ptr->_children;
      }
    }
      break;
    case NodeType::ObjectExpression:
    {
      DumpAst((GASTNode*)root->_value, level + 1);
    }
      break;
    case NodeType::Property:
    {
      GTypeTraits<NodeType::Property>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::Property>::type*>(root->_value);
      printLine("|- key: {}\n", ptr->key(), level);
      printLine("|- value:\n", "", level);
      DumpAst(ptr->value(), level + 1);
    }
      break;
    case NodeType::QueryStatement:
    {
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
      break;
    case NodeType::BinaryExpression:
    {
      GTypeTraits<NodeType::BinaryExpression>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::BinaryExpression>::type*>(root->_value);
      printLine("|- name: {}\n", ptr->key(), level);
      printLine("|- value:\n", "", level);
      DumpAst(ptr->value(), level + 1);
    }
      break;
    case NodeType::VertexDeclaration:
    {
      GTypeTraits<NodeType::VertexDeclaration>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::VertexDeclaration>::type*>(root->_value);
      DumpAst(ptr->vertex(), level + 1);
    }
      break;
    case NodeType::EdgeDeclaration:
    {
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
      break;
    default:
      break;
  }
  for (size_t idx = 0; idx < root->_size; ++idx) {
    DumpAst(root->_children + idx, level + 1);
  }
}