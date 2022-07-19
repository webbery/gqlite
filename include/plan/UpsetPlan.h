#pragma once
#include "Plan.h"
#include <variant>
#include "base/lang/lang.h"
#include "base/lang/AST.h"

struct GASTNode;
class GUpsetPlan: public GPlan {
public:
  GUpsetPlan(GVirtualNetwork* vn, GStorageEngine* store, GUpsetStmt* stmt);

  virtual int prepare();
  virtual int execute(gqlite_callback);

private:
  struct UpsetVisitor {
    GUpsetPlan& _plan;

    UpsetVisitor(GUpsetPlan& plan): _plan(plan) {}
    VisitFlow apply(GASTNode* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path) {
      std::string graph = stmt->name();
      return VisitFlow::Children;
    }
    VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GGQLExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GProperty* stmt, std::list<NodeType>& path) {
      std::string k = stmt->key();
      accept(stmt->value(), *this, path);
      return VisitFlow::Children;
    }
    VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path) {
      _plan._vertex = true;
      GLiteral* literal = (GLiteral*)(stmt->key()->_value);
      switch (literal->kind()) {
      case AttributeKind::Number:
        _plan._key = atoi(literal->raw().c_str());
        break;
      default:
        _plan._key = literal->raw();
        break;
      }
      accept(stmt->vertex(), *this, path);
      return VisitFlow::Children;
    }
    VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path) {
      for (auto itr = stmt->begin(), end = stmt->end(); itr != end; ++itr) {
        accept(*itr, *this, path);
      }
      return VisitFlow::Children;
    }
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path) {
      _plan._vertex = false;
      return VisitFlow::Children;
    }
    VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;

    }
  };

  friend class UpsetVisitor;
private:
  bool _vertex;       /**< true if upset target is vertex, else is edge */
  std::string _class;
  std::variant<std::string, uint64_t> _key;
  std::string _value;
};