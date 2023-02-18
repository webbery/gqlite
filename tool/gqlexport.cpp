#include "../include/gqlite.h"
#include <stdio.h>

void usage() {
  printf("dump graph to gremlin. Usage:\n");
  printf("\tgqldump db_file\n");
}

int main(int argc, char** argv) {
  // if (argc < 2) return -1;
  gqlite* pHandle = 0;
  gqlite_open(&pHandle, "gql_db");
  printf("not implement\n");
  gqlite_statement* statement = 0;
  gqlite_create(pHandle, "{dump: 'ga'}", &statement);
  printf("33333\n");
  gqlite_execute(pHandle, statement);
  printf("4444444\n");
  gqlite_close(pHandle);
  printf("55555s\n");
  return 0;
}
