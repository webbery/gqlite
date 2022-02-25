#include <iostream>
#include <cassert>
#include <string>
#include "../include/VirtualEngine.h"
#ifndef GQL_STRING
#include "../src/gql.h"
#include "../src/gql.c"
#endif

#define SQL_COUNT   1

struct Command {
    int cmd;
    Token tk;
};

int main() {
  Token tk = {0};
  Command commands[] = {
    {GQL_SHOW, tk},
    {GQL_GRAPH, tk},
    {0, 0}
  };

  int yv;
  int count = sizeof(commands)/sizeof(Command);
  void* _pParser = ParseAlloc(malloc);
  for (int i = 0; i<count; ++i) {
    Parse(_pParser, yv, tk);
  }
  Parse(_pParser, NEWLINE, tk);
  Parse(_pParser, yv, tk);
  ParseFree(_pParser, free);
}