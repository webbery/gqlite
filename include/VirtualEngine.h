#pragma once
#include <stdint.h>
#include <string>
#include "base/MemoryPool.h"
#include "SubGraph.h"

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
class GVirtualEngine {
public:
  static uint32_t GenerateIndex();

  GVirtualEngine();
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
  GSubGraph* _graph = nullptr;
  GStorageEngine* _storage = nullptr;
private:
  static uint32_t _indx;
  MemoryPool<char> _memory;
};