#include "gqlite.h"
#include "GQliteImpl.h"
#include <atomic>
#include "Statement.h"
#include "Error.h"

#define CHECK_NULL_PTR(p) {if (!p) return ECODE_NULL_PTR;}

std::atomic<bool> _gqlite_g_close_flag_(false);

SYMBOL_EXPORT int gqlite_open_with_mode(const char* filename, gqlite** ppDb, gqlite_open_mode mode)
{
  GQLiteImpl* impl = new GQLiteImpl();
  *ppDb = (gqlite*)impl;
  return impl->open(filename, mode);
}

SYMBOL_EXPORT int gqlite_open(const char* filename, gqlite** ppDb)
{
  gqlite_open_mode mode;
  mode.st_schema = gqlite_disk;
  return gqlite_open_with_mode(filename, ppDb, mode);
}

SYMBOL_EXPORT int gqlite_close(gqlite* gql)
{
  _gqlite_g_close_flag_.store(true);
  GQLiteImpl* ptr = (GQLiteImpl*)gql;
  delete ptr;
  return 0;
}

SYMBOL_EXPORT int gqlite_exec(gqlite* pDb, const char* gql, int (*gqlite_callback)(gqlite_result*), void*, char** err)
{
  CHECK_NULL_PTR(pDb);
  GStatement stm(gql, gqlite_callback);
  GQLiteImpl* impl = (GQLiteImpl*)pDb;
  int ret = impl->exec(stm);
  if (ECode_Success != ret) {

  }
  return ret;
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

SYMBOL_EXPORT const char* gqlite_error(int error)
{
  return 0;
}
