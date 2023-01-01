#include "gqlite.h"
#include "GQliteImpl.h"
#include <atomic>
#include "VirtualEngine.h"
#include "Error.h"
#include "Memory.h"
#include "json.hpp"
#include "StorageEngine.h"
#include "gutil.h"

#define MAX_MEMORY  5*1024*1024
#define CHECK_NULL_PTR(p) {if (!p) return ECODE_NULL_PTR;}

#define UNKNOWN_ERROR           "unknow error"
#define Graph_Not_Exist_ERROR   "graph is not exist"
#define Group_Not_Exist_ERROR   "group is not exist"
#define Index_Not_Exist_ERROR   "index %s is not exist"
#define GRAMMAR_ARRAY_ERROR     "input array seems not correct"

std::atomic<bool> _gqlite_g_close_flag_(false);

namespace {
  std::string get_operation(GQL_Command_Type type, const std::string& gql) {
    switch (type) {
    case GQL_Creation: return "CREATE";
    case GQL_Query: return "QUERY";
    case GQL_Upset: return "UPSET";
    case GQL_Drop: return "DROP";
    case GQL_Remove: return "REMOVE";
    default:
      return "EXECUTE COMMAND";
    }
  }

  char* simple_message(const char* message) {
    size_t len = strlen(message) + 1;
    char* msg = (char*)GMemory::allocate(len);
    strncpy(msg, message, len);
    return msg;
  }
}

SYMBOL_EXPORT int gqlite_open_with_mode(const char* filename, gqlite** ppDb, gqlite_open_mode mode)
{
  GVirtualEngine* stm = new GVirtualEngine(MAX_MEMORY);
  GQLiteImpl* impl = new GQLiteImpl(stm);
  *ppDb = (gqlite*)impl;
  return impl->open(filename, mode);
}

SYMBOL_EXPORT int gqlite_open(gqlite** ppDb, const char* filename)
{
  gqlite_open_mode mode;
  if (filename) {
    mode.st_schema = gqlite_disk;
  } else {
    mode.st_schema = gqlite_memory;
  }
  return gqlite_open_with_mode(filename, ppDb, mode);
}

SYMBOL_EXPORT int gqlite_close(gqlite* gql)
{
  _gqlite_g_close_flag_.store(true);
  GQLiteImpl* ptr = (GQLiteImpl*)gql;
  delete ptr;
  return 0;
}

SYMBOL_EXPORT int gqlite_exec(gqlite* pDb, const char* gql, int (*gqlite_callback)(gqlite_result*, void*), void* handle, char** err)
{
  CHECK_NULL_PTR(pDb);
  GQLiteImpl* impl = (GQLiteImpl*)pDb;
  GVirtualEngine* stm = impl->engine();
  stm->_errIndx = 0;
  stm->_result_callback = gqlite_callback;
  stm->_gql = gql;
  stm->_handle = handle;
  stm->_errorCode = 0;
  impl->exec(*stm);
  char* msg = gqlite_error(pDb, stm->_errorCode);
  *err = msg;
  return stm->_errorCode;
}

SYMBOL_EXPORT int gqlite_version(gqlite* ppDb, int* major, int* minor, int* patch) {
  GQLiteImpl* impl = (GQLiteImpl*)ppDb;
  if (!impl) return 0;
  GVirtualEngine* stm = impl->engine();
  if (!stm || !stm->_storage) return 0;
  auto pStorage = stm->_storage;
  if (!pStorage->isOpen()) return 0;
  auto schema = pStorage->getSchema();
  std::string version = schema[SCHEMA_GLOBAL][GLOBAL_GQL_VERSION];
  auto versions = gql::split(version.c_str(), '.');
  *major = atoi(versions[0].c_str());
  *minor = atoi(versions[1].c_str());
  *patch = atoi(versions[2].c_str());
  return 1;
}

SYMBOL_EXPORT int gqlite_create(gqlite* pDb, const char* gql, gqlite_statement** statement)
{
  CHECK_NULL_PTR(pDb);
  GQLiteImpl* impl = (GQLiteImpl*)pDb;
  return 0;
}

SYMBOL_EXPORT int gqlite_execute(gqlite* pDb, gqlite_statement* statement)
{
  CHECK_NULL_PTR(pDb);
  return 0;
}

SYMBOL_EXPORT char* gqlite_error(gqlite* pDb, int error)
{
  char* msg = nullptr;
  size_t len = 0;
  char buff[256] = { 0 };
  GQLiteImpl* impl = (GQLiteImpl*)pDb;
  GVirtualEngine* stm = impl->engine();
  std::string gql = stm->gql();
  std::string operation = get_operation(stm->_cmdtype, gql);
  switch (error)
  {
  case ECode_Success:
    operation += " SUCCESS";
    msg = simple_message(operation.c_str());
    break;
  case ECode_Graph_Not_Exist:
  {
    msg = simple_message(Graph_Not_Exist_ERROR);
  }
    break;
  case ECode_GQL_Index_Not_Exist:
    //sprintf(buff, Index_Not_Exist_ERROR, jsn["index"].dump().c_str());
    break;
  case ECode_Group_Not_Exist:
    msg = simple_message(Group_Not_Exist_ERROR);
    break;
  case ECode_GQL_Parse_Fail:
    break;
  case GQL_GRAMMAR_ARRAY_FAIL:
    msg = simple_message(GRAMMAR_ARRAY_ERROR);
    break;
  case ECode_Fail:
  default:
    msg = simple_message(UNKNOWN_ERROR);
    break;
  }
  return msg;
}

SYMBOL_EXPORT void gqlite_free(void* ptr)
{
  GMemory::deallocate(ptr);
}
