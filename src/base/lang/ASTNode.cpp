#include <map>
#include <functional>
#include <string>
#include <fmt/printf.h>
#include <cassert>
#include "base/lang/AST.h"
#include "base/lang/ASTNode.h"
#include "base/lang/visitor/IVisitor.h"
#include "base/type.h"

#define RETURN_CASE_NODE_TYPE(node_type) case NodeType::node_type: return #node_type
#define RETURN_CASE_PROP_KIND(prop_kind) case AttributeKind::prop_kind: return #prop_kind
#define RUN_VisitFlow_Children(action) {\
  switch (vf) {\
  case VisitFlow::Return: return vf;\
  case VisitFlow::Children:\
    vf = accept(value->action, visitor, path);\
    break;\
  case VisitFlow::SkipCurrent:\
    vf = pre;\
    break;\
  default:\
    break;\
  }\
}

#define DELETE_OBJECT(nodetype) \
  case NodeType::nodetype:\
  {\
    GTypeTraits<NodeType::nodetype>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::nodetype>::type*>(node->_value);\
    delete ptr;\
  }\
  break

void FreeNodeImpl(GListNode* node) {
  switch (node->_nodetype)
  {
    DELETE_OBJECT(CreationStatement);
    DELETE_OBJECT(UpsetStatement);
    DELETE_OBJECT(QueryStatement);
    DELETE_OBJECT(WalkDeclaration);
    DELETE_OBJECT(VertexDeclaration);
    DELETE_OBJECT(EdgeDeclaration);
    DELETE_OBJECT(DropStatement);
    DELETE_OBJECT(Literal);
    DELETE_OBJECT(GQLExpression);
    DELETE_OBJECT(ArrayExpression);
    DELETE_OBJECT(RemoveStatement);
    DELETE_OBJECT(ObjectExpression);
    DELETE_OBJECT(Property);
    DELETE_OBJECT(BinaryExpression);
    DELETE_OBJECT(CallExpression);
    DELETE_OBJECT(GroupStatement);
    DELETE_OBJECT(MemberExpression);
    DELETE_OBJECT(DumpStatement);
    DELETE_OBJECT(LambdaExpression);
    DELETE_OBJECT(BlockStatement);
    DELETE_OBJECT(VariableDeclaration);
    DELETE_OBJECT(AssignStatement);
  case NodeType::VariableDeclarator:
  {
    GListNode* ptr = (GListNode*)node->_value;
    FreeNode(ptr);
  }
    break;
  case NodeType::ConditionStatement: {
    GListNode* ptr = (GListNode*)node->_value;
    FreeNode(ptr);
  }
  default:
    break;
  }
  delete node;
}

GListNode* MakeNode(enum NodeType type, void* value, GListNode* children) {
  GListNode* ast = new GListNode;
  ast->_value = value;
  ast->_nodetype = type;
  ast->_children = children;
  return ast;
}

void FreeNode(GListNode* root) {
  if (!root) return;
  if (!root->_value) return;
  if (root->_children) {
    FreeNodeImpl(root->_children);
  }
  FreeNodeImpl(root);
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
    RETURN_CASE_NODE_TYPE(LambdaExpression);
    RETURN_CASE_NODE_TYPE(AssignStatement);
  default: return "Unknow Node Type: " + std::to_string((int)nt);
  }
}

std::string PropertyKind2String(AttributeKind kind) {
  switch (kind) {
    RETURN_CASE_PROP_KIND(Binary);
    RETURN_CASE_PROP_KIND(Number);
    RETURN_CASE_PROP_KIND(Integer);
    RETURN_CASE_PROP_KIND(String);
  default: return "Unknow Property Kind: " + std::to_string((int)kind);
  }
}

void printLine(const std::string& format, const std::string& str, size_t level) {
  for (size_t lvl = 0; lvl < level; ++lvl) {
    fmt::printf(": ");
  }
  fmt::print(format, str);
}

std::string GetString(GListNode* node) {
  GLiteral* str = (GLiteral*)node->_value;
  return str->raw();
}

std::vector<double> GetVector(GListNode* node) {
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

VisitFlow GViewVisitor::apply(GListNode* stmt, std::list<NodeType>& path) {
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
    accept(*itr, this, path);
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

VisitFlow GViewVisitor::apply(GLambdaExpression* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(LambdaExpression), level);
  return VisitFlow::Children;
}

VisitFlow GViewVisitor::apply(GReturnStmt* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(ReturnStatement), level);
  return VisitFlow::Children;
}

VisitFlow GViewVisitor::apply(GBlockStmt* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(BlockStatement), level);
  return VisitFlow::Children;
}

VisitFlow GViewVisitor::apply(GVariableDecl* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(VariableDeclaration), level);
  printLine("|- name: {}\n", stmt->name(), level);
  return VisitFlow::Children;
}

VisitFlow GViewVisitor::apply(GDropStmt* stmt, std::list<NodeType>& path)
{
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(DropStatement), level);
  return VisitFlow::Children;
}

VisitFlow GViewVisitor::apply(GBinaryExpression* stmt, std::list<NodeType>& path) {
  accept(stmt->left(), this, path);
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(BinaryExpression), level);
  auto op = [](uint8_t o){
    switch ((GBinaryExpression::Operator)o) {
    case GBinaryExpression::Operator::Add: return "+";
    case GBinaryExpression::Operator::Subtract: return "-";
    case GBinaryExpression::Operator::Multiply: return "*";
    case GBinaryExpression::Operator::Divide: return "/";
    default: return "unknow operator";
    }
  };
  
  printLine("|- type: {}\n", op(stmt->getOperator()), level);
  accept(stmt->right(), this, path);
  return VisitFlow::Children;
}

VisitFlow GViewVisitor::apply(GObjectFunction* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(CallExpression), level);
  return VisitFlow::Children;
}

VisitFlow GViewVisitor::apply(GAssignStmt* stmt, std::list<NodeType>& path) {
  size_t level = path.size();
  printLine("|- type: {}\n", NodeType2String(AssignStatement), level);
  printLine("|- name: {}\n", stmt->name(), level);
  return VisitFlow::Children;
}

VisitFlow accept(GListNode* node, GVisitor* visitor, std::list<NodeType>& path) {
  if (node == nullptr) return VisitFlow::Children;
  path.push_back(node->_nodetype);
  VisitFlow vf = VisitFlow::Children;
  switch(node->_nodetype) {
    case NodeType::GQLExpression: {
      vf = visitor->apply(node, path);
      while (node && vf == VisitFlow::Children) {
        node = node->_children;
        vf = accept(node, visitor, path);
      }
    }
    break;
    case NodeType::CreationStatement:
    {
      GTypeTraits<NodeType::CreationStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::CreationStatement>::type*>(node->_value);
      vf = visitor->apply(ptr, path);
      switch(vf) {
        case VisitFlow::Children:
        {
          GArrayExpression* groups = (GArrayExpression*)ptr->groups()->_value;
          for (GListNode* group: *groups) {
            GGroupStmt* stmt = (GGroupStmt*)group->_value;
            GListNode* list = stmt->indexes();
            VisitFlow state = accept(list, visitor, path);
            if (state != VisitFlow::Children) return state;
          }
        }
        break;
        case VisitFlow::Return: vf = VisitFlow::Return;
        default: break;
      }
    }
    break;
    case NodeType::UpsetStatement:
    {
      GTypeTraits<NodeType::UpsetStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::UpsetStatement>::type*>(node->_value);
      vf = visitor->apply(ptr, path);
    }
    break;
    case NodeType::VertexDeclaration:
    {
      GListNode* ptr = node;
      while (ptr)
      {
        GTypeTraits<NodeType::VertexDeclaration>::type* value = reinterpret_cast<GTypeTraits<NodeType::VertexDeclaration>::type*>(ptr->_value);
        VisitFlow pre = vf;
        vf = visitor->apply(value, path);
        RUN_VisitFlow_Children(vertex());
        ptr = ptr->_children;
      }
    }
    break;
    case NodeType::Literal:
    {
      GTypeTraits<NodeType::Literal>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::Literal>::type*>(node->_value);
      vf = visitor->apply(ptr, path);
    }
    break;
    case NodeType::ArrayExpression:
    {
      GTypeTraits<NodeType::ArrayExpression>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::ArrayExpression>::type*>(node->_value);
      VisitFlow pre = vf;
      vf = visitor->apply(ptr, path);
      switch(vf) {
        case VisitFlow::Return: return vf;
        case VisitFlow::Children:
        {
          auto itr = ptr->begin();
          while(itr != ptr->end()) {
            accept(*itr, visitor, path);
            ++itr;
          }
        }
        break;
        case VisitFlow::SkipCurrent:
          vf = pre;
          break;
        default:
        break;
      }
    }
    break;
    case NodeType::QueryStatement:
    {
      GTypeTraits<NodeType::QueryStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::QueryStatement>::type*>(node->_value);
      vf = visitor->apply(ptr, path);
      accept(ptr->graph(), visitor, path);
      if (ptr->where()) {
        accept(ptr->where(), visitor, path);
      }
    }
    break;
    case NodeType::ObjectExpression:
    case NodeType::Property:
    {
      GTypeTraits<NodeType::Property>::type* value = reinterpret_cast<GTypeTraits<NodeType::Property>::type*>(node->_value);
      VisitFlow pre = vf;
      vf = visitor->apply(value, path);
      RUN_VisitFlow_Children(value());
    }
    break;
    case NodeType::BinaryExpression:
    {
      GTypeTraits<NodeType::BinaryExpression>::type* value = reinterpret_cast<GTypeTraits<NodeType::BinaryExpression>::type*>(node->_value);
      VisitFlow pre = vf;
      vf = visitor->apply(value, path);
      switch (vf) {
      case VisitFlow::Return: return vf;
      case VisitFlow::Children:
        break;
      case VisitFlow::SkipCurrent:
        vf = pre;
        break;
      default:
          break;
      }
    }
    break;
    case NodeType::EdgeDeclaration:
    {
      GTypeTraits<NodeType::EdgeDeclaration>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::EdgeDeclaration>::type*>(node->_value);
      vf = visitor->apply(ptr, path);
    }
      break;
    case NodeType::WalkDeclaration:
    {
      GTypeTraits<NodeType::WalkDeclaration>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::WalkDeclaration>::type*>(node->_value);
      vf = visitor->apply(ptr, path);
    }
      break;
    case NodeType::DropStatement:
    {
      GTypeTraits<NodeType::DropStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::DropStatement>::type*>(node->_value);
      vf = visitor->apply(ptr, path);
    }
    break;
    case NodeType::DumpStatement:
    {
      GTypeTraits<NodeType::DumpStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::DumpStatement>::type*>(node->_value);
      vf = visitor->apply(ptr, path);
    }
    break;
    case NodeType::RemoveStatement:
    {
      GTypeTraits<NodeType::RemoveStatement>::type* value = reinterpret_cast<GTypeTraits<NodeType::RemoveStatement>::type*>(node->_value);
      vf = visitor->apply(value, path);
    }
    break;
    case NodeType::CallExpression: {
      GTypeTraits<NodeType::CallExpression>::type* value = reinterpret_cast<GTypeTraits<NodeType::CallExpression>::type*>(node->_value);
      vf = visitor->apply(value, path);
    }
    break;
    case NodeType::BlockStatement: {
      GTypeTraits<NodeType::BlockStatement>::type* value = reinterpret_cast<GTypeTraits<NodeType::BlockStatement>::type*>(node->_value);
      VisitFlow pre = vf;
      vf = visitor->apply(value, path);
      RUN_VisitFlow_Children(block());
    }
    break;
    case NodeType::LambdaExpression: {
      GTypeTraits<NodeType::LambdaExpression>::type* value = reinterpret_cast<GTypeTraits<NodeType::LambdaExpression>::type*>(node->_value);
      VisitFlow pre = vf;
      vf = visitor->apply(value, path);
      RUN_VisitFlow_Children(block());
    }
    break;
    case NodeType::ReturnStatement: {
      GTypeTraits<NodeType::ReturnStatement>::type* value = reinterpret_cast<GTypeTraits<NodeType::ReturnStatement>::type*>(node->_value);
      VisitFlow pre = vf;
      vf = visitor->apply(value, path);
      RUN_VisitFlow_Children(expr());
    }
    break;
    case NodeType::VariableDeclaration: {
      GTypeTraits<NodeType::VariableDeclaration>::type* value = reinterpret_cast<GTypeTraits<NodeType::VariableDeclaration>::type*>(node->_value);
      VisitFlow pre = vf;
      vf = visitor->apply(value, path);
    }
    break;
    case NodeType::AssignStatement: {
      GTypeTraits<NodeType::AssignStatement>::type* value = reinterpret_cast<GTypeTraits<NodeType::AssignStatement>::type*>(node->_value);
      VisitFlow pre = vf;
      vf = visitor->apply(value, path);
      RUN_VisitFlow_Children(value());
    }
    break;
    case NodeType::ConditionStatement: {
      GListNode* value = (GListNode*)node->_value;
      vf = visitor->apply(value, path);
    }
    default: vf = visitor->apply(node, path); break;
  }
  path.pop_back();
  return vf;
}
