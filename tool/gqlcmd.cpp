#include <iostream>
#include <chrono>
#ifdef WIN32
#include <conio.h>
#pragma warning(disable : 4054)
#pragma warning(disable : 4055)
#pragma warning(disable : 4100)
#pragma warning(disable : 4127)
#pragma warning(disable : 4130)
#pragma warning(disable : 4152)
#pragma warning(disable : 4189)
#pragma warning(disable : 4206)
#pragma warning(disable : 4210)
#pragma warning(disable : 4232)
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)
#pragma warning(disable : 4306)
#pragma warning(disable : 4702)
#pragma warning(disable : 4706)
#else
#include <unistd.h>
#include <termios.h>
#endif
#include <vector>
#include <string>
#include "gqlite.h"
#include "linenoise.hpp"
#include "stdout.h"
#define MAX_HISTORY_SIZE  100
#define HISTORY_FILENAME  ".gql_history"

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

  std::string input;
  gqlite* pHandle = 0;
  gqlite_open(dbfile.c_str(), &pHandle);
  do {
    bool quit = linenoise::Readline("gqlite> ", input);
    if (quit || input == "exit") break;
    linenoise::AddHistory(input.c_str());
    linenoise::SaveHistory(HISTORY_FILENAME);
    char* ptr = nullptr;
    auto start = std::chrono::high_resolution_clock::now();
    int error = gqlite_exec(pHandle, input.c_str(), gqlite_exec_callback, nullptr, &ptr);
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    if (ptr) {
      auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()/1000.0;
      printf("%s, COST %gs\n", ptr, microseconds);
      gqlite_free(ptr);
    }
  } while (true);
  gqlite_close(pHandle);
  return 0;
}