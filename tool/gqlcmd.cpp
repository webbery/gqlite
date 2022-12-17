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
#pragma warning(disable : 4819)
#else
#include <unistd.h>
#include <termios.h>
#endif
#include <vector>
#include <string>
#include "gqlite.h"
#include "linenoise.h"
#include "stdout.h"
#include "base/Debug.h"
#define MAX_HISTORY_SIZE  100
#define HISTORY_FILENAME  ".gql_history"

int main(int argc, char** argv) {
#ifdef __linux__
  init_coredump_capture();
#endif
  std::string dbfile = "";
  if (argc > 1) {
    dbfile = argv[1];
  }
  linenoiseInstallWindowChangeHandler();
  linenoiseHistoryLoad(HISTORY_FILENAME);
  linenoiseSetCompletionCallback([](char const* prefix, linenoiseCompletions* lc) {

  });

  std::string input;
  gqlite* pHandle = 0;
  int ret = 0;
  if (dbfile.size()) {
    ret = gqlite_open(&pHandle, dbfile.c_str());
  } else {
    ret = gqlite_open(&pHandle);
  }
  if (ret) return ret;
  printf("GQLite Version %s\nWritting by Webberg.\n\nMIT License. Build: %s\n", PROJECT_VERSION, GIT_REVISION_SHA);
  do {
    char* result = linenoise("gqlite> ");
    if (result == nullptr) break;
    std::string input(result);
    free(result);
    if (*result == '\0' || input == "exit") break;
    linenoiseHistoryAdd(input.c_str());
    char* ptr = nullptr;
    auto start = std::chrono::high_resolution_clock::now();
    int error = gqlite_exec(pHandle, input.c_str(), gqlite_exec_callback, nullptr, &ptr);
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    if (ptr) {
      auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000000.0;
      printf("%s, COST %gs\n", ptr, microseconds);
      gqlite_free(ptr);
    }
  } while (true);
  linenoiseHistorySave(HISTORY_FILENAME);
  linenoiseHistoryFree();
  gqlite_close(pHandle);
  return 0;
}