#include "../include/gqlite.h"
#include <stdio.h>

int main(int argc, char** argv) {
  // if (argc < 2) return -1;
  gqlite* pHandle = 0;
  printf("11111111\n");
  gqlite_open(&pHandle, "gql_db");
  printf("22222\n");
  gqlite_statement* statement = 0;
  gqlite_create(pHandle, "{dump: 'ga'}", &statement);
  printf("33333\n");
  gqlite_execute(pHandle, statement);
  printf("4444444\n");
  gqlite_close(pHandle);
  printf("55555s\n");
  return 0;
}
