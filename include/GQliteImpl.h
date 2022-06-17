#pragma once
#include "VirtualEngine.h"
#include "gqlite.h"

enum gqlite_storage_schema {
  gqlite_disk,
  gqlite_memory,
};
typedef struct _gqlite_open_mode {
  gqlite_storage_schema st_schema;
}gqlite_open_mode;


class GQueryEngine;
class GStorageEngine;

class GQLiteImpl {
public:
  GQLiteImpl(GVirtualEngine* );
  ~GQLiteImpl();

  int open(const char* filename, gqlite_open_mode mode);

  void exec(GVirtualEngine& stm);

  int close();
  
  GVirtualEngine* statement() { return _statement; }
private:
  int create(const char* filename, gqlite_open_mode mode);

private:
  GVirtualEngine* _statement = nullptr;
};