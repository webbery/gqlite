#include "base/lang/lang.h"

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

/**
 * @brief This is used to visit an AST with custom warpper.
 * 
 * @tparam _Wrap 
 */
class GAST{
public:
  GAST(GASTNode* node) : _root(node) {}
  template <typename Visitor>
  void accept(const Visitor& visitor) {

    switch(node->_nodetype) {
      case NodeType::GQLExpression: visitor.apply(node); break;
      case NodeType::CreationStatement:
      {
        GTypeTraits<NodeType::CreationStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::CreationStatement>::type*>(node->_value);
        visitor.apply(ptr);
        GASTNode* list = ptr->indexes();
        while (list) {
          GASTNode* node = (GASTNode*)list->_value;
          accept(node);
          list = list->_children;
        }
      }
      break;
      case NodeType::UpsetStatement:
      {
        GTypeTraits<NodeType::UpsetStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::UpsetStatement>::type*>(node->_value);
        _w.apply(ptr);
      }
      break;
      case NodeType::VertexDeclaration:
      {
        GTypeTraits<NodeType::VertexDeclaration>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::VertexDeclaration>::type*>(node->_value);
        _w.apply(ptr);
      }
      break;
      default: _w.apply(node); break;
    }
  }

private:
  GASTNode* _root;
};