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

  int exec(GStatement& stm);

  int close();
  
private:
  int create(const char* filename, gqlite_open_mode mode);

private:
  GVirtualEngine* _pVirtualEngine = nullptr;
};