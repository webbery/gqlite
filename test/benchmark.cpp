/**********************************************************
 * Read test file for benchmark.
 * Test file format looks like as follows:
 * run times; one graph script; expect cost(time/path cost and so on);
 * 
 * If you are not care about expect cost, you can set it empty.
 * In order to caculate average cost, 
 * 
 * 
 * Usage:
 *   benchmark test_file [dbfile]
 **********************************************************/
#include <string>
#include <string.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "gqlite.h"

std::string g_dbfile;

int gqlite_benchmark_callback(gqlite_result* params, void*)
{
  if (params) {
    switch (params->type)
    {
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    return -1;
  }

  std::filesystem::path inputs = std::filesystem::current_path();
  std::string g_inputdir = inputs.u8string();
  std::filesystem::path bench_path(argv[1]);
  if (bench_path.is_relative()) {
    bench_path = inputs/bench_path;
  }

  gqlite* gHandle = nullptr;
  if (argc == 3) {
    std::string dbfile(argv[2]);
    std::filesystem::path p(dbfile);
    if (p.is_relative()) {
      g_dbfile = g_inputdir + dbfile;
    }
    gqlite_open(&gHandle, g_dbfile.c_str());
  } else {
    gqlite_open(&gHandle);
  }

  #define LINE_MAX_SIZE 1024
  char gql[LINE_MAX_SIZE] = { 0 };

  std::ifstream fs;
  fs.open(bench_path.u8string().c_str(), std::ios_base::in);
  int lineno = 0;
  while (fs.getline(gql, LINE_MAX_SIZE)) {
    char* ptr = nullptr;
    if (strlen(gql) == 0) break;

    
    int value = gqlite_exec(gHandle, gql, gqlite_benchmark_callback, nullptr, &ptr);
  }
  gqlite_close(gHandle);
  return 0;
}