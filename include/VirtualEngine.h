#pragma once
#include <stdint.h>
#include <string>
#include "base/MemoryPool.h"
#include "base/lang/AST.h"

struct _gqlite_result;

typedef int (*gqlite_callback)(_gqlite_result*);

enum GQL_Command_Type {
  GQL_Creation,
  GQL_Query,
  GQL_Upset,
  GQL_Drop,
  GQL_Remove,
  GQL_Util,
  GQL_Command_Size
};

class GStorageEngine;
struct GASTNode;
class GPlan;
class GVirtualNetwork;
class GVirtualEngine {
public:
  static uint32_t GenerateIndex();

  GVirtualEngine(size_t);
  GVirtualEngine( const char* gql, gqlite_callback cb);
  ~GVirtualEngine();
  
  const std::string& gql() const {return _gql;}

  void* alloc(size_t size);

  int execAST(GASTNode* ast);

  gqlite_callback _result_callback;
  std::string _gql;
  size_t _errIndx;
  // result code of sql executing result
  int _errorCode;
  // message of sql executing result
  //std::string _msg;
  // gql type
  GQL_Command_Type _cmdtype;
  GStorageEngine* _storage = nullptr;

private:
  struct PlanVisitor {
    GPlan* _plan = nullptr;
    GVirtualNetwork* _vn;
    GStorageEngine* _store;
    PlanVisitor(GVirtualNetwork* vn, GStorageEngine* store):_vn(vn), _store(store){}
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
      return VisitFlow::Children;
    }
    VisitFlow apply(GProperty* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path);
    VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path);
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
  };
private:
  GPlan* makePlans(GASTNode* ast);
  int executePlans(GPlan*);
  void cleanPlans(GPlan*);

private:
  static uint32_t _indx;
  MemoryPool<char> _memory;
  GVirtualNetwork* _network;
};