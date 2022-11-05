#include <map>
#include <functional>
#include <string>
#include <fmt/printf.h>
#include <cassert>
#include "base/lang/AST.h"

#define RETURN_CASE_NODE_TYPE(node_type) case NodeType::node_type: return #node_type
#define RETURN_CASE_PROP_KIND(prop_kind) case AttributeKind::prop_kind: return #prop_kind
#define FREE_NODE(node) case 

void FreeNode(GASTNode* node) {
  switch (node->_nodetype)
  {
  case NodeType::CreationStatement:
  {
    GTypeTraits<NodeType::CreationStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::CreationStatement>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::UpsetStatement:
  {
    GTypeTraits<NodeType::UpsetStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::UpsetStatement>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::QueryStatement:
  {
    GTypeTraits<NodeType::QueryStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::QueryStatement>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::WalkDeclaration:
  {
    GTypeTraits<NodeType::WalkDeclaration>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::WalkDeclaration>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::VertexDeclaration:
  {
    GTypeTraits<NodeType::VertexDeclaration>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::VertexDeclaration>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::EdgeDeclaration:
  {
    GTypeTraits<NodeType::EdgeDeclaration>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::EdgeDeclaration>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::DropStatement:
  {
    GTypeTraits<NodeType::DropStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::DropStatement>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::Literal:
  {
    GTypeTraits<NodeType::Literal>::type* ptr = static_cast<GTypeTraits<NodeType::Literal>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::GQLExpression:
  {
    GTypeTraits<NodeType::GQLExpression>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::GQLExpression>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::ArrayExpression:
  {
    GTypeTraits<NodeType::ArrayExpression>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::ArrayExpression>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::RemoveStatement:
  {
    GTypeTraits<NodeType::RemoveStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::RemoveStatement>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::ObjectExpression:
  {
    GTypeTraits<NodeType::ObjectExpression>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::ObjectExpression>::type*>(node->_value);
    FreeNode(ptr);
  }
    break;
  case NodeType::Property:
  {
    GTypeTraits<NodeType::Property>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::Property>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::BinaryExpression:
  {
    GTypeTraits<NodeType::BinaryExpression>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::BinaryExpression>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::VariableDeclarator:
  {
    GASTNode* ptr = (GASTNode*)node->_value;
    FreeNode(ptr);
  }
    break;
  case NodeType::CallExpression:
  {
    GASTNode* ptr = (GASTNode*)node->_value;
    FreeNode(ptr);
  }
    break;
  case NodeType::GroupStatement:
  {
    GTypeTraits<NodeType::GroupStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::GroupStatement>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::MemberExpression:
  {
    GTypeTraits<NodeType::MemberExpression>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::MemberExpression>::type*>(node->_value);
    delete ptr;
  }
    break;
  case NodeType::DumpStatement:
  {
    GTypeTraits<NodeType::DumpStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::DumpStatement>::type*>(node->_value);
    delete ptr;
  }
  break;
  default:
    break;
  }
  delete node;
}

GASTNode* NewAst(enum NodeType type, void* value, GASTNode* children, size_t size) {
  GASTNode* ast = new GASTNode;
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
  FreeNode(root);
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
    RETURN_CASE_NODE_TYPE(WalkDeclaration);
    RETURN_CASE_NODE_TYPE(DropStatement);
    RETURN_CASE_NODE_TYPE(RemoveStatement);
  default: return "Unknow Node Type: " + std::to_string((int)nt);
  }
}

std::string PropertyKind2String(AttributeKind kind) {
  switch (kind) {
    RETURN_CASE_PROP_KIND(Binary);
    RETURN_CASE_PROP_KIND(Number);
    RETURN_CASE_PROP_KIND(String);
  default: return "Unknow Property Kind: " + std::to_string((int)kind);
  }
}

void printLine(const std::string& format, const std::string& str, size_t level) {
  for (size_t lvl = 0; lvl < level; ++lvl) {
    fmt::printf("  ");
  }
  fmt::print(format, str);
}

std::string GetString(GASTNode* node) {
  GLiteral* str = (GLiteral*)node->_value;
  return str->raw();
}

attribute_t GetLiteral(GASTNode* node)
{
  attribute_t v;
  if (node->_nodetype != NodeType::Literal) return v;
  GLiteral* literal = (GLiteral*)node->_value;
  switch (literal->kind()) {
  case AttributeKind::String:
    v = literal->raw();
    break;
  case AttributeKind::Datetime:
    v = (double)atoll(literal->raw().c_str());
    break;
  case AttributeKind::Integer:
  case AttributeKind::Number:
    v = (double)atof(literal->raw().c_str());
    break;
  default:
    break;
  }
  return v;
};

std::vector<double> GetVector(GASTNode* node) {
  std::vector<double> v;
  if (node->_nodetype != NodeType::ArrayExpression) return v;
  GArrayExpression* arr = (GArrayExpression*)node->_value;
  for (auto itr = arr->begin(); itr != arr->end(); ++itr) {
    GLiteral* literal = (GLiteral*)(*itr)->_value;
    double d = (double)atof(literal->raw().c_str());
    v.push_back(d);
  }
  return v;
}

VisitFlow GViewVisitor::apply(GASTNode* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("`- type: {}\n", NodeType2String(stmt->_nodetype), level);
  // printLine("|- name: {}\n", stmt->name(), level);
  return VisitFlow::Children;
}
VisitFlow GViewVisitor::apply(GUpsetStmt* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(UpsetStatement), level);
  printLine("|- name: {}\n", stmt->name(), level);
  return VisitFlow::Children;
}
VisitFlow GViewVisitor::apply(GQueryStmt* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(QueryStatement), level);
  printLine("|- query:\n", "", level);
  // DumpAst(ptr->query(), level + 1);
  printLine("|- graph:\n", "", level);
  return VisitFlow::Children;
}
VisitFlow GViewVisitor::apply(GGQLExpression* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(GQLExpression), level);
  // printLine("|- name: {}\n", stmt->name(), level);
  return VisitFlow::Children;
}
VisitFlow GViewVisitor::apply(GProperty* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(Property), level);
  printLine("|- key: {}\n", stmt->key(), level);
  printLine("|- value:\n", "", level);
  return VisitFlow::Children;
}
VisitFlow GViewVisitor::apply(GVertexDeclaration* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(VertexDeclaration), level);
  return VisitFlow::Children;
}
VisitFlow GViewVisitor::apply(GCreateStmt* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(CreationStatement), level);
  printLine("|- name: {}\n", stmt->name(), level);
  return VisitFlow::Children;
}
VisitFlow GViewVisitor::apply(GLiteral* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(Literal), level);
  printLine("|- raw: {}\n", stmt->raw(), level);
  printLine("|- kind: {}\n", PropertyKind2String(stmt->kind()), level);
  return VisitFlow::Children;
}

VisitFlow GViewVisitor::apply(GArrayExpression* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(ArrayExpression), level);
  auto itr = stmt->begin();
  size_t cnt = 1;
  while (itr != stmt->end()) {
    printLine("|- #{}\n", std::to_string(cnt++), level);
    accept(*itr, *this, path);
    ++itr;
  }
  return VisitFlow::SkipCurrent;
}

VisitFlow GViewVisitor::apply(GEdgeDeclaration* stmt, std::list<NodeType>& path)
{
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(EdgeDeclaration), level);
  path.push_back(NodeType::InvalidNode);
  printLine("|- link:{}\n", "", level+1);
  //accept(stmt->link(), *this, path);
  //printLine("|- from:{}\n", "", level+1);
  //accept(stmt->from(), *this, path);
  //printLine("|- to:{}\n", "", level+1);
  //accept(stmt->to(), *this, path);
  path.pop_back();
  return VisitFlow::Children;
}

VisitFlow GViewVisitor::apply(GDropStmt* stmt, std::list<NodeType>& path)
{
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(DropStatement), level);
  return VisitFlow::Children;
}

VisitFlow GViewVisitor::apply(GObjectFunction* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(CallExpression), level);
  return VisitFlow::Children;
}