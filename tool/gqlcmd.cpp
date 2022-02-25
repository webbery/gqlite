#include <iostream>
#ifdef WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif
#include <vector>
#include <string>
#include "gqlite.h"
#include "linenoise.hpp"
#define MAX_HISTORY_SIZE  100
#define HISTORY_FILENAME  ".gql_history"

int gqlite_exec_callback(gqlite_result* results) {
  if (results && results->count) {
    std::cout<<"result count: "<<results->count<<std::endl;
  }
  return 0;
}

int main(int argc, char** argv) {
  std::string dbfile = "default.gdb";
  if (argc > 1) {
    dbfile = argv[1];
  }
  // enable multiline
  linenoise::SetMultiLine(true);
  linenoise::LoadHistory(HISTORY_FILENAME);
  linenoise::SetHistoryMaxLen(MAX_HISTORY_SIZE);
  linenoise::SetCompletionCallback([](const char* editBuffer, std::vector<std::string>& completions)
    {
      //if (editBuffer[0] == '\n') {
      //}
    }
  );

  char err[512] = { 0 };
  char* ptr = err;
  std::string input;
  gqlite* pHandle = 0;
  gqlite_open(dbfile.c_str(), &pHandle);
  do {
    bool quit = linenoise::Readline("gqlite> ", input);
    if (quit || input == "exit") break;
    linenoise::AddHistory(input.c_str());
    linenoise::SaveHistory(HISTORY_FILENAME);
    gqlite_exec(pHandle, input.c_str(), gqlite_exec_callback, nullptr, &ptr);
  } while (true);
  gqlite_close(pHandle);
  return 0;
}