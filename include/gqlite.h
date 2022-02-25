#pragma once
#include <stdint.h>

#ifdef _WIN32
#define SYMBOL_EXPORT
#else
#define SYMBOL_EXPORT __attribute__((visibility("default")))
#endif

typedef void* gqlite;

typedef struct _gqlite_statement {

}gqlite_statement;

enum gqlite_storage_schema {
  gqlite_disk,
  gqlite_memory,
};
typedef struct _gqlite_open_mode {
  gqlite_storage_schema st_schema;
}gqlite_open_mode;

enum gqlite_primitive_type {
  gqlite_int,
  gqlite_string,
  gqlite_decimal,
  gqlite_array,
  gqlite_object
};

typedef struct _gqlite_node {
  // nodeid, C-style string
  char* id;
  // node properties
  char* properties;
  uint32_t len;
}gqlite_node;

typedef struct _gqlite_edge {
  char* id;
  char* properties;
  uint32_t len;
}gqlite_edge;

enum gqlite_result_type {
  gqlite_node_type,
  gqlite_edge_type,
  gqlite_name_type
};
typedef struct _gqlite_result {
  union {
    _gqlite_node* nodes;
    _gqlite_edge* edges;
    char** graphs;
  };
  uint32_t count;
  gqlite_result_type type;
}gqlite_result;

#ifdef __cplusplus
extern "C" {
#endif

  SYMBOL_EXPORT int gqlite_open(const char* filename, gqlite** ppDb);
  SYMBOL_EXPORT int gqlite_open_with_mode(const char* filename, gqlite** ppDb, gqlite_open_mode mode);
  SYMBOL_EXPORT int gqlite_exec(gqlite* pDb, const char* gql, int (*gqlite_callback)(gqlite_result*), void*, char** err);
  SYMBOL_EXPORT int gqlite_create(gqlite* pDb, const char* gql, gqlite_statement** statement);
  SYMBOL_EXPORT int gqlite_execute(gqlite* pDb, gqlite_statement* statement);
  SYMBOL_EXPORT int gqlite_next(gqlite* pDb, gqlite_statement* statement, gqlite_result** result);

SYMBOL_EXPORT int gqlite_close(gqlite*);
SYMBOL_EXPORT const char* gqlite_error(int error);

#ifdef __cplusplus
}
#endif
