#pragma once
#include "VirtualEngine.h"
#include "gqlite.h"
#include "Statement.h"

class GQueryEngine;
class GStorageEngine;

class GQLiteImpl {
public:
  GQLiteImpl();
  ~GQLiteImpl();

  int open(const char* filename, gqlite_open_mode mode);

  void exec(GStatement& stm);

  int close();
  
  void set(GStatement* pStatement);
  GStatement* statement() { return _statement; }
private:
  int create(const char* filename, gqlite_open_mode mode);

private:
  GStatement* _statement = nullptr;
  GVirtualEngine* _pVirtualEngine = nullptr;
};