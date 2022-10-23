#pragma once
#include "base/lang/lang.h"
#include <list>
#include <functional>

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

template<> struct GTypeTraits<NodeType::DropStatement> {
  typedef GDropStmt type;
};

template <> struct GTypeTraits<NodeType::UpsetStatement> {
  typedef GUpsetStmt type;
};

template <> struct GTypeTraits<NodeType::ArrayExpression> {
  typedef GArrayExpression type;
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

template <> struct GTypeTraits<NodeType::WalkDeclaration> {
  typedef GWalkDeclaration type;
};

template <> struct GTypeTraits<NodeType::RemoveStatement> {
  typedef GRemoveStmt type;
};

template <> struct GTypeTraits<NodeType::ObjectExpression> {
  typedef GASTNode type;
};

template <> struct GTypeTraits<NodeType::GroupStatement> {
  typedef GGroupStmt type;
};

template <> struct GTypeTraits<NodeType::MemberExpression> {
  typedef GMemberExpression type;
};

template <> struct GTypeTraits<NodeType::DumpStatement> {
  typedef GDumpStmt type;
};

/******************************
 * accept function will visit all nodes start from input node.
 * We define ourself's visitor to process nodes when type is matched.
 * And parameter of path show current tree path of node type.
 ******************************/
template <typename Visitor>
VisitFlow accept(GASTNode* node, Visitor& visitor, std::list<NodeType>& path) {
  if (node == nullptr) return VisitFlow::Children;
  path.push_back(node->_nodetype);
  VisitFlow vf = VisitFlow::Children;
  switch(node->_nodetype) {
    case NodeType::GQLExpression: visitor.apply(node, path); break;
    case NodeType::CreationStatement:
    {
      GTypeTraits<NodeType::CreationStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::CreationStatement>::type*>(node->_value);
      vf = visitor.apply(ptr, path);
      switch(vf) {
        case VisitFlow::Children:
        {
          GASTNode* list = ptr->indexes();
          VisitFlow state = accept(list, visitor, path);
          if (state != VisitFlow::Children) return state;
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
      vf = visitor.apply(ptr, path);
    }
    break;
    case NodeType::VertexDeclaration:
    {
      GASTNode* ptr = node;
      while (ptr)
      {
        GTypeTraits<NodeType::VertexDeclaration>::type* value = reinterpret_cast<GTypeTraits<NodeType::VertexDeclaration>::type*>(ptr->_value);
        vf = visitor.apply(value, path);
        switch(vf) {
          case VisitFlow::Children:
          {
            accept(value->vertex(), visitor, path);
          }
          break;
          default: break;
        }
        ptr = ptr->_children;
      }
    }
    break;
    case NodeType::Literal:
    {
      GTypeTraits<NodeType::Literal>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::Literal>::type*>(node->_value);
      vf = visitor.apply(ptr, path);
    }
    break;
    case NodeType::ArrayExpression:
    {
      GTypeTraits<NodeType::ArrayExpression>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::ArrayExpression>::type*>(node->_value);
      vf = visitor.apply(ptr, path);
      switch(vf) {
        case VisitFlow::Children:
        {
          auto itr = ptr->begin();
          while(itr != ptr->end()) {
            accept(*itr, visitor, path);
            ++itr;
          }
        }
        break;
        default:
        break;
      }
    }
    break;
    case NodeType::QueryStatement:
    {
      GTypeTraits<NodeType::QueryStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::QueryStatement>::type*>(node->_value);
      vf = visitor.apply(ptr, path);
      accept(ptr->graph(), visitor, path);
      if (ptr->where()) {
        accept(ptr->where(), visitor, path);
      }
    }
    break;
    case NodeType::BinaryExpression:
    case NodeType::Property:
    {
      GTypeTraits<NodeType::Property>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::Property>::type*>(node->_value);
      vf = visitor.apply(ptr, path);
      switch (vf)
      {
      case VisitFlow::Children:
        vf = accept(ptr->value(), visitor, path);
        break;
      default:
        break;
      }
    }
    break;
    case NodeType::ObjectExpression:
    {
      vf = visitor.apply((GASTNode*)node->_value, path);
      if (vf == VisitFlow::Children) {
        vf = accept((GASTNode*)node->_value, visitor, path);
      }
    }
    break;
    case NodeType::EdgeDeclaration:
    {
      GTypeTraits<NodeType::EdgeDeclaration>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::EdgeDeclaration>::type*>(node->_value);
      vf = visitor.apply(ptr, path);
    }
      break;
    case NodeType::WalkDeclaration:
    {}
      break;
    case NodeType::DropStatement:
    {
      GTypeTraits<NodeType::DropStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::DropStatement>::type*>(node->_value);
      vf = visitor.apply(ptr, path);
    }
    break;
    case NodeType::DumpStatement:
    {
      GTypeTraits<NodeType::DumpStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::DumpStatement>::type*>(node->_value);
      vf = visitor.apply(ptr, path);
    }
    break;
    case NodeType::RemoveStatement:
    {
      GTypeTraits<NodeType::RemoveStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::RemoveStatement>::type*>(node->_value);
      vf = visitor.apply(ptr, path);
    }
    break;
    default: vf = visitor.apply(node, path); break;
  }
  for (size_t idx = 0; idx < node->_size; ++idx) {
    accept(node->_children + idx, visitor, path);
  }
  path.pop_back();
  return vf;
}

/******************************************************
 * GeneralVisitor support default behavior for visiting all of AST nodes.
 * If you want to implement special visitor for some kind of nodes, you can inherit this
 * class and rewrite some member function in it.
 * ***************************************************/
struct GeneralVisitor {
  VisitFlow apply(GASTNode* stmt, std::list<NodeType>& path) {
    return VisitFlow::Children;
  }
  VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path) {
    return VisitFlow::Children;
  }
  VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path) {
    return VisitFlow::Children;
  }
  VisitFlow apply(GGQLExpression* stmt, std::list<NodeType>& path) {
    return VisitFlow::Children;
  }
  VisitFlow apply(GProperty* stmt, std::list<NodeType>& path) {
    return VisitFlow::Children;
  }
  VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path) {
    return VisitFlow::Children;
  }
  VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path) {
    return VisitFlow::Children;
  }
  VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path) {
    return VisitFlow::Children;
  }
  VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path) {
    return VisitFlow::Children;
  }
};
