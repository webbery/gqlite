#pragma once
#include "Plan.h"
#include <variant>
#include <string>
#include "base/lang/ASTNode.h"

struct GASTNode;
class GVirtualNetwork;
class GStorageEngine;
class GUtilPlan: public GPlan {
public:
  enum class UtilType {
    Creation,
  };
  GUtilPlan(GVirtualNetwork* vn, GStorageEngine* store, GASTNode* ast);
  virtual int execute();

private:
  struct CreationVisitor {
    GUtilPlan& _plan;
    CreationVisitor(GUtilPlan& plan): _plan(plan) {}
    VisitFlow apply(GASTNode* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GGQLExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GProperty* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path) {
      _plan._var = stmt->name();
      return VisitFlow::Children;
    }
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
  };

  friend class CreationVisitor;
private:
  GVirtualNetwork* _vn;
  GStorageEngine* _store;

  UtilType _type;
  std::variant<std::string> _var;
};