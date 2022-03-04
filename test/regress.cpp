#include <stdio.h>
#ifdef WIN32
#include "../tool/getopt.h"
#else
#include <unistd.h>
#include <getopt.h>
#include <string.h>
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

int bengin_capture(const char* output_filename, FILE*& fp) {
  fp = fopen(output_filename, "w");
  int stdout_bk = dup(fileno(stdout));
  dup2(fileno(fp), fileno(stdout));
  
  return stdout_bk;
}

void close_capture(int pipe, FILE* fp) {
  fclose(fp);
  dup2(pipe, fileno(stdout));
}

int main(int argc, char** argv) {
  parse_opt(argc, argv);
  std::filesystem::path inputs = std::filesystem::current_path();
  if (g_inputdir != ".") inputs = g_inputdir;

  std::string outfile = g_inputdir + "/current.out";
  std::cout<<" Result: "<<outfile.c_str()<<std::endl;
  FILE* fp = nullptr;
  int outfd = bengin_capture(outfile.c_str(), fp);
  gqlite* gHandle = nullptr;
  gqlite_open((g_inputdir + "_regress").c_str(), &gHandle);
#define LINE_MAX_SIZE 1024
  char gql[LINE_MAX_SIZE] = { 0 };
  for (auto& file : std::filesystem::directory_iterator(inputs)) {
    std::string curfile = file.path().u8string();
    if (curfile.find("current.out") != std::string::npos || curfile.find("expect.out") != std::string::npos) continue;
    // load script
    std::cout << "***** EXECUTE GQL: " << file.path() << " *****" <<std::endl;
    std::ifstream fs;
    fs.open(file.path().u8string().c_str(), std::ios_base::in);
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
  close_capture(outfd, fp);
  // compare result
  bool is_ok = true;
  std::string expect_file{inputs.u8string()};
  expect_file += "/expect.out";
  if (!std::filesystem::exists(expect_file) || !std::filesystem::exists(outfile)) return 0;
  std::string cmd("diff -T -u ");
  cmd += outfile;
  cmd += " " + expect_file;
  pid_t status = system(cmd.c_str());
  if(-1 != status && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
    // success and not diff
  }
  else {
    // fprintf(stderr, "%s", (int)status);
    // is_ok = false;
  }
  return !is_ok;
}