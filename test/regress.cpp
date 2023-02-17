#include <stdio.h>
#ifdef WIN32
#include <io.h>
#include "../tool/getopt.h"
#define SPLASH_WORD "\\"
#else
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#define SPLASH_WORD "/"
#endif
#include <string>
#include <filesystem>
#include <fstream>
#include "gqlite.h"
#include "../tool/stdout.h"
#include "base/Debug.h"

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
std::string g_dbfile;

enum opt_type {
  inputdir = 1,
  outputdir,
  load_ext,
  dbfile,
  dlpath,
  version
};

static struct option lopts[] = {
  {"inputdir",        required_argument,  0, inputdir},
  {"outputdir",       required_argument,  0, outputdir},
  {"load-extension",  required_argument,  0, load_ext},
  {"dbfile",          required_argument,  0, dbfile},
  {"dlpath",          required_argument,  0, dlpath},
  {"version",         no_argument,        0, version},
  {0, 0, 0, 100}
};

static void print_usage() {
  int idx = 0;
  while (help[idx]) {
    printf("%s", help[idx]);
    idx++;
  }
}

static int parse_opt(int argc, char** argv) {
  char* arg = nullptr;
  int c = 0;
  int opt_index = -1;
  int ret = -1;
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
    case dbfile:
      g_dbfile = optarg;
      break;
    case dlpath:
      break;
    case version:
      printf("gqlite version: %s\n", REGRESS_VERSION);
      break;
    default:
      print_usage();
      ret = 0;
      break;
    }
  }
  return ret;
}

int bengin_capture(const char* output_filename, FILE*& fp) {
#ifdef WIN32
  fp = fopen(output_filename, "w");
  int stdout_bk = _dup(_fileno(stdout));
  _dup2(_fileno(fp), _fileno(stdout));
  return stdout_bk;
#else
  // int stdout_bk = dup(fileno(stdout));
  // dup2(fileno(fp), fileno(stdout));
  fp = freopen(output_filename, "w", stdout);
  return 0;
#endif
}

void close_capture(int pipe, FILE* fp) {
#ifdef WIN32
  fclose(fp);
  dup2(pipe, fileno(stdout));
#else
  fclose(fp);
#endif
}

int main(int argc, char** argv) {
  if (!parse_opt(argc, argv)) return -1;
#ifdef __linux__
  init_coredump_capture();
#endif
  std::filesystem::path inputs = std::filesystem::current_path();
  if (g_inputdir != ".") inputs = g_inputdir;
  if (!inputs.is_absolute()) {
    inputs = std::filesystem::absolute(inputs);
    g_inputdir = inputs.u8string();
  }

  std::string outfile = g_inputdir + SPLASH_WORD +"current.out";
  FILE* fp = nullptr;
  int outfd = bengin_capture(outfile.c_str(), fp);
  gqlite* gHandle = nullptr;
  if (g_dbfile.size()) {
    std::filesystem::path p(g_dbfile);
    if (p.is_relative()) {
      g_dbfile = g_inputdir + g_dbfile;
    }
    gqlite_open(&gHandle, g_dbfile.c_str());
  } else {
    gqlite_open(&gHandle);
  }
#define LINE_MAX_SIZE 1024
  char gql[LINE_MAX_SIZE] = { 0 };
  for (auto& file : std::filesystem::directory_iterator(inputs)) {
    std::string curfile = file.path().u8string();
    if (curfile.find("current.out") != std::string::npos || curfile.find("expect.out") != std::string::npos) continue;
    // load script
    printf("***** EXECUTE: \"%s\" *****\n", file.path().filename().c_str());
    std::ifstream fs;
    fs.open(file.path().u8string().c_str(), std::ios_base::in);
    int lineno = 0;
    while (fs.getline(gql, LINE_MAX_SIZE)) {
      char* ptr = nullptr;
      if (strlen(gql) == 0) break;
      // execute script
      printf("[%d]:\t%s\n", lineno, gql);
      int value = gqlite_exec(gHandle, gql, gqlite_exec_callback, nullptr, &ptr);
      char* msg = gqlite_error(gHandle, value);
      if (msg) {
        printf("%s\n", msg);
        gqlite_free(msg);
      }
      lineno += 1;
      memset(gql, 0, LINE_MAX_SIZE);
    }
    printf("***** EXECUTE FINISH: \"%s\" *****\n", file.path().filename().c_str());
  }
  gqlite_close(gHandle);
  close_capture(outfd, fp);
  // compare result
  bool is_ok = true;
  std::string expect_file{ inputs.u8string() };
  
  expect_file = expect_file + SPLASH_WORD + "expect.out";
  if (!std::filesystem::exists(expect_file) || !std::filesystem::exists(outfile)) return 0;
#ifdef WIN32
  std::string cmd("fc /N ");
#else
  std::string cmd("diff -T -u ");
#endif
  cmd += outfile;
  cmd += " " + expect_file;
#ifdef WIN32
  int status
#else
  pid_t status
#endif
    = system(cmd.c_str());
  if(
#ifdef WIN32
    status == 0
#else
    - 1 != status && WIFEXITED(status) && WEXITSTATUS(status) == 0
#endif
    ) {
    // success and not diff
  }
  else {
#ifdef WIN32
    printf("Run Command: %s\n", cmd.c_str());
    if (status == -1) {
      switch (errno) {
      case E2BIG:
      case ENOENT:
      case ENOEXEC:
      case ENOMEM:
        break;
      default:
        break;
      }
    }
    else {
      is_ok = false;
    }
#else
    // fprintf(stderr, "%s", (int)status);
// is_ok = false;
#endif
  }
  return !is_ok;
}