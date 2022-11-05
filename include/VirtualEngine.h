#pragma once
#include <stdint.h>
#include <string>
#include <map>
#include "base/MemoryPool.h"
#include "base/lang/AST.h"

// error code of parsing gql  
#define GQL_GRAMMAR_ARRAY_FAIL    -256
#define GQL_GRAMMAR_OBJ_FAIL      -257

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
  static const char* GetErrorInfo(int code);

  GVirtualEngine(size_t);
  GVirtualEngine( const char* gql, gqlite_callback cb);
  ~GVirtualEngine();
  
  const std::string& gql() const {return _gql;}

  void* alloc(size_t size);

  /**
   * @brief parse an AST, then execute it.
   * 
   * @param ast an input AST from yacc
   * @return int 
   */
  int execAST(GASTNode* ast);

  /**
   * @brief execute some simple command that have no complex ast
   * 
   * @param ast an input AST from yacc
   * @return int 
   */
  int execCommand(GASTNode* ast);

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
  struct PlanList {
    GPlan* _plan;
    /**< true if plan can be run in thread and next plan did not wait it finish. */
    bool _threadable;
    PlanList* _next;
    PlanList* _parent;
  };
  struct PlanVisitor {
    PlanList* _plans = nullptr;
    std::map<std::string, GVirtualNetwork*>& _vn;
    GStorageEngine* _store;
    gqlite_callback _cb;
    PlanVisitor(std::map<std::string, GVirtualNetwork*>& vn, GStorageEngine* store, gqlite_callback cb = nullptr):_vn(vn), _store(store){
      _plans = new PlanList;
      _plans->_next = _plans;
      _plans->_parent = _plans;
      _plans->_plan = nullptr;
      _plans->_threadable = false;
      _cb = cb;
    }

    /**
     * Add a plan to list
     */
    void add(GPlan* plan, bool threadable = false);

    VisitFlow apply(GASTNode* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path);
    VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path);
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
    VisitFlow apply(GDumpStmt* stmt, std::list<NodeType>& path);
    VisitFlow apply(GRemoveStmt* stmt, std::list<NodeType>& path);
    VisitFlow apply(GObjectFunction* stmt, std::list<NodeType>& path);
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GWalkDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
  };
private:
  PlanList* makePlans(GASTNode* ast);
  int executePlans(PlanList*);
  void cleanPlans(PlanList*);

private:
  MemoryPool<char> _memory;
  std::map<std::string, GVirtualNetwork*> _networks;
};