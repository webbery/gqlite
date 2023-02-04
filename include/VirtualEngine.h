#pragma once
#include <stdint.h>
#include <string>
#include <map>
#include "base/MemoryPool.h"
#include "base/gvm/Chunk.h"
#include "base/gvm/Value.h"
#include "base/gvm/Compiler.h"
#include "base/lang/AST.h"
#include "base/gvm/GVM.h"
#include "base/lang/ASTNode.h"
#include "base/lang/BlockStmt.h"
#include "base/lang/GQLExpression.h"
#include "base/lang/VariableDecl.h"

// error code of parsing gql  
#define GQL_GRAMMAR_ARRAY_FAIL    -256
#define GQL_GRAMMAR_OBJ_FAIL      -257

struct _gqlite_result;

typedef int (*gqlite_callback)(_gqlite_result*, void*);

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
struct GListNode;
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
  int execAST(GListNode* ast);

  /**
   * @brief execute some simple command that have no complex ast
   * 
   * @param ast an input AST from yacc
   * @return int 
   */
  int execCommand(GListNode* ast);

  /**
   * @brief For graph script, this function emit byte code to chunk.
   *        Then gvm will envoke the byte code and other plan will retrieve the result.
   */
  void emit(uint8_t byte);

  gqlite_callback _result_callback;
  std::string _gql;
  void* _handle;

  size_t _errIndx;

  // result code of sql executing result
  int _errorCode;

  // gql type
  GQL_Command_Type _cmdtype;
  GStorageEngine* _storage = nullptr;

  // Record gscript count of `{` and `}`.
  // _scriptRangeCnt++ if `{` increase,  other wise decrease if encounter `}`
  int _scriptRangePairCnt = 0;

private:
  struct PlanList {
    GPlan* _plan;
    /**< true if plan can be run in thread and next plan did not wait it finish. */
    bool _threadable;
    PlanList* _next;
    PlanList* _parent;
  };
  struct PlanVisitor: public GVisitor {
    PlanList* _plans = nullptr;
    Chunk* _chunk = nullptr;
    std::map<std::string, GVirtualNetwork*>& _vn;
    GStorageEngine* _store;
    GVM* _gvm;
    gqlite_callback _cb;
    void* _handle;
    PlanVisitor(std::map<std::string, GVirtualNetwork*>& vn, GVM* gvm, GStorageEngine* store, gqlite_callback cb = nullptr, void* handle = nullptr)
      :_vn(vn), _store(store), _handle(handle), _gvm(gvm) {
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

    VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path);
    VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path);

    VisitFlow apply(GProperty* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
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
    VisitFlow apply(GLambdaExpression* stmt, std::list<NodeType>& path);
    VisitFlow apply(GReturnStmt* stmt, std::list<NodeType>& path);
  };

  // read ast and generate byte code
  struct ByteCodeVisitor: public GVisitor {
    Chunk* _currentChunk = nullptr;
    Compiler* _compiler = nullptr;
    GVM* _gvm = nullptr;

    ByteCodeVisitor(GVM* gvm):_currentChunk(new Chunk), _compiler(new Compiler), _gvm(gvm) {
      
    }

    VisitFlow apply(GGQLExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GProperty* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }

    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path);

    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GWalkDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GLambdaExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GReturnStmt* stmt, std::list<NodeType>& path);
    VisitFlow apply(GBlockStmt* stmt, std::list<NodeType>& path);
    VisitFlow apply(GBinaryExpression* stmt, std::list<NodeType>& path);
    VisitFlow apply(GVariableDecl* stmt, std::list<NodeType>& path);

    /**
    * @brief For graph script, this function emit byte code to chunk.
    *        Then gvm will envoke the byte code and other plan will retrieve the result.
    */
    void emit(uint8_t byte);
    
    template<typename... Types>
    void emit(uint8_t byte, Types... args) {
      if (!_currentChunk) {
        _currentChunk = new Chunk;
      }
      _currentChunk->_code.push_back(byte);
      emit(args...);
    }
  };
private:
  PlanList* makePlans(GListNode* ast);
  int executePlans(PlanList*);
  void cleanPlans(PlanList*);

private:
  MemoryPool<char> _memory;
  std::map<std::string, GVirtualNetwork*> _networks;

  GVM* _gvm = nullptr;
};