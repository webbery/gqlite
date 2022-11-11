#include <catch.hpp>
#include <fstream>
#include <stdio.h>
#include <string>
#include "gqlite.h"
#include "../tool/stdout.h"

void readCSV(const std::string& name, std::function<void(char*)> cb, bool skip_head = true) {
  std::string csv = _WORKING_DIR_ "/data/ml-latest-small/" + name;
  std::ifstream fs;
  fs.open(csv);
  size_t line_num = 0;
  while (fs && !fs.eof())
  {
    char buff[512] = { 0 };
    fs.getline(buff, 512);
    if (skip_head) {
      skip_head = false;
      continue;
    }
    cb(buff);
  }
  fs.close();
}

TEST_CASE("init movies") {
  gqlite* pHandle = 0;
  gqlite_open(&pHandle);

  int major, minor, patch;
  assert(gqlite_version(pHandle, &major, &minor, &patch) == 0);

  char* ptr = nullptr;
  gqlite_exec(pHandle,
    "{drop: 'movielens_db'};",
    gqlite_exec_callback, nullptr, &ptr);
  gqlite_free(ptr);
  gqlite_exec(pHandle,
    "{create: 'movielens_db',"
      "group: ["
        "{movie: ['title', 'genres']},"
        "{tag: ['user_id', 'tag', 'movie_id']},"
        "{rate: ['user_id', 'rate', 'movie_id']}"
      "]"
    "};",
    gqlite_exec_callback, nullptr, &ptr);
  assert(gqlite_version(pHandle, &major, &minor, &patch) > 0);
  gqlite_free(ptr);
  readCSV("movies.csv", [&pHandle, &ptr](char* buffer) {
    char* movie_id = strtok(buffer, ",");
    if (movie_id == nullptr) return;
    int id = atoi(movie_id);
    char* movie_title = strtok(nullptr, ",");
    std::string title = replace_all(movie_title);
    char* movie_genres = strtok(nullptr, ",");
    std::string genres = replace_all(movie_genres);

    char upset[512] = { 0 };
    sprintf(upset, "{upset: 'movie', vertex: [[%d, {title: '%s', genres: '%s'}]]};", id, title.c_str(), genres.c_str());
    gqlite_exec(pHandle, upset, gqlite_exec_callback, nullptr, &ptr);
    gqlite_free(ptr);
    });
  int line_num = 1;
  readCSV("tags.csv", [&pHandle, &ptr, &line_num](char* buffer) {
    char* user_id = strtok(buffer, ",");
    if (user_id == nullptr) return;
    int uid = atoi(user_id);
    char* movie_id = strtok(nullptr, ",");
    int mid = atoi(movie_id);
    char* ctag = strtok(nullptr, ",");
    std::string tag = replace_all(ctag);
    char upset[512] = { 0 };
    sprintf(upset, "{upset: 'tag', edge: [[%d, {--: {tag: '%s'} }, %d]]};", uid, tag.c_str(), mid);
    gqlite_exec(pHandle, upset, gqlite_exec_callback, nullptr, &ptr);
    gqlite_free(ptr);
    line_num++;
    });
  gqlite_exec(pHandle,
   "{query: 'movie', in: 'movielens_db'};",
   gqlite_exec_callback, nullptr, &ptr);
  gqlite_free(ptr);
  gqlite_exec(pHandle,
    "{query: 'tag', in: 'movielens_db'};",
    gqlite_exec_callback, nullptr, &ptr);
  gqlite_free(ptr);
  readCSV("ratings.csv", [&pHandle, &ptr, &line_num](char* buffer) {
    });
  gqlite_exec(pHandle, "{dump: 'movielens_db'};", gqlite_exec_callback, nullptr, &ptr);
  gqlite_free(ptr);
  gqlite_close(pHandle);
}
