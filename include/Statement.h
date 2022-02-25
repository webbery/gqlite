#pragma once
#include <stdint.h>
#include <string>
#include "base/MemoryPool.h"

struct _gqlite_result;

typedef int (*gqlite_callback)(_gqlite_result*);

class GStatement {
public:
  static uint32_t GenerateIndex();

  GStatement( const char* gql, gqlite_callback cb);
  
  const std::string& gql() const {return _gql;}

  void* alloc(size_t size);

  gqlite_callback _result_callback;
  size_t _errIndx;
private:
  static uint32_t _indx;
  std::string _gql;
  MemoryPool<char> _memory;
};