#include <stdio.h>
#ifdef WIN32
#include "../tool/getopt.h"
#else
#include <unistd.h>
#endif
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "gqlite.h"

#define REGRESS_VERSION   "0.1.0"

static const char* help[] = {
  "options:\n",
  "--inputdir=DIR         take input files from DIR (default \".\")\n",
  "--outputdir=DIR        place output files in DIR (default \".\")\n",
  "--load-extension=EXT   load the named extension before running the tests; can appear multiple times\n",
  "--dlpath=DIR           look for dynamic libraries in DIR\n",
  "--version              output version information, then exit\n",
  0
};

std::string g_inputdir = ".";
std::string g_outputdir = ".";

enum opt_type {
  inputdir = 1,
  outputdir,
  load_ext,
  dlpath,
  version
};

static struct option lopts[] = {
  {"inputdir",        required_argument,  0, inputdir},
  {"outputdir",       required_argument,  0, outputdir},
  {"load-extension",  required_argument,  0, load_ext},
  {"dlpath",          required_argument,  0, dlpath},
  {"version",         no_argument,        0, version},
  {0, 0, 0, 100}
};

static void print_usage() {
  int idx = 0;
  while (help[idx]) {
    printf(help[idx]);
    idx++;
  }
}

static void parse_opt(int argc, char** argv) {
  char* arg = nullptr;
  int c = 0;
  int opt_index = -1;
  while ((c = getopt_long(argc, argv, "h", lopts, &opt_index)) != -1) {
    switch (c)
    {
    case inputdir:
      g_inputdir = optarg;
      break;
    case outputdir:
      g_outputdir = optarg;
      break;
    case load_ext:
      break;
    case dlpath:
      break;
    case version:
      printf("gqlite version: %s\n", REGRESS_VERSION);
      break;
    default:
      print_usage();
      break;
    }
  }
}

int main(int argc, char** argv) {
  parse_opt(argc, argv);
  std::filesystem::path inputs = std::filesystem::current_path();
  if (g_inputdir != ".") inputs = g_inputdir;
  gqlite* gHandle = nullptr;
  gqlite_open((g_inputdir + "_regress").c_str(), &gHandle);
#define LINE_MAX_SIZE 1024
  char gql[LINE_MAX_SIZE] = { 0 };
  for (auto& file : std::filesystem::directory_iterator(inputs)) {
    // load script
    std::cout << "***** EXECUTE GQL: " << file.path() << " *****" <<std::endl;
    std::ifstream fs;
    fs.open(file.path(), std::ios_base::in);
    while (fs.getline(gql, LINE_MAX_SIZE)) {
      char err[256] = { 0 };
      char* ptr = err;
      // execute script
      gqlite_exec(gHandle, gql, [](gqlite_result*)-> int {
        return 0;
        }, nullptr, &ptr);
      memset(gql, 0, LINE_MAX_SIZE);
    }
    std::cout << "***** EXECUTE FINISH: " << file.path() << " *****" << std::endl;
  }
  gqlite_close(gHandle);
  return 0;
}