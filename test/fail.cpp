#include <catch.hpp>
#include "gqlite.h"
#include "../tool/stdout.h"

TEST_CASE("init movies") {
  gqlite* pHandle = 0;
  gqlite_open(&pHandle);
  gqlite_open(&pHandle);
  char* ptr = nullptr;
  gqlite_exec(pHandle,
    "{query: 'e', in: 'ga'};",
    gqlite_exec_callback, pHandle, &ptr
  );
  gqlite_free(ptr);
  gqlite_close(pHandle);
}
