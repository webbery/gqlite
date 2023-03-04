/**********************************************************
 * Read test file for benchmark.
 * Test file format looks like as follows:
 * run times; one graph script; expect cost(time(unit: microsecond)/path/memory cost and so on);
 * 
 * If you are not care about expect cost, you can set it empty.
 * In order to caculate average cost, 
 * 
 * 
 * Usage:
 *   benchmark test_file [dbfile]
 **********************************************************/
#include <cassert>
#include <string>
#include <string.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <map>
#include "gqlite.h"

std::string g_dbfile;

std::string split_by(const std::string& input, char delim) {
  std::string data(input);
  size_t pos = 0;
  while ((pos = data.find(delim, pos)) != std::string::npos) {
      return data.substr(0, pos + 1);
  }
  return data;
}

std::map<std::string, std::string> parse_check(const std::string& input) {
  std::map<std::string, std::string> checks;
  std::string check_info(input);
  std::string check_item;
  while ((check_item = split_by(check_info, ',')).size()) {
    std::string key = split_by(check_item, ':');
    std::string value = check_item.substr(key.size(), check_item.size());
    checks[key.substr(0, key.size() - 1)] = value;
    check_info = check_info.substr(check_item.size());
  }
  return checks;
}

int gqlite_benchmark_callback(gqlite_result* params, void*)
{
  if (params) {
    switch (params->type)
    {
    }
  }
  return 0;
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
    if (!std::filesystem::is_regular_file(bench_path)) {
      printf("%s is not a normal file.\n", bench_path.u8string().c_str());
      return -1;
    }
  }

  gqlite* gHandle = nullptr;
  if (argc == 3) {
    std::string dbfile(argv[2]);
    std::filesystem::path p(dbfile);
    if (p.is_relative()) {
      g_dbfile = g_inputdir + "/" + dbfile;
    }
    if (!std::filesystem::exists(g_dbfile)) {
      printf("db file not exist here: %s\n", g_dbfile.c_str());
      return -1;
    }
    gqlite_open(&gHandle, g_dbfile.c_str());
  } else {
    gqlite_open(&gHandle, nullptr);
  }

  std::string gql;

  std::ifstream fs;
  fs.open(bench_path.u8string().c_str(), std::ios_base::in);
  int lineno = 0;
  while (std::getline(fs, gql)) {
    char* ptr = nullptr;
    if (gql.empty()) break;
    std::string s_cnt = split_by(gql, ';');
    std::string query = split_by(gql.substr(s_cnt.size(), gql.size()), ';');
    std::string checks = split_by(gql.substr(s_cnt.size() + query.size()), ';');
    auto check_items = parse_check(checks);
    int times = std::atoi(s_cnt.substr(0, s_cnt.size() - 1).c_str());
    printf("exec: %s\n",query.c_str());
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < times; ++i) {
      int value = gqlite_exec(gHandle, query.c_str(), gqlite_benchmark_callback, nullptr, &ptr);
    }
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    if (check_items.count("time")) {
      auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000000.0;
      auto average = microseconds / times;
      double thresold = atof(check_items["time"].c_str()) / 1000;
      printf("cost: %f, expect: %f\n", average, thresold);
      assert(average <= thresold);
    }
  }
  printf("exit\n");
  gqlite_close(gHandle);
  return 0;
}