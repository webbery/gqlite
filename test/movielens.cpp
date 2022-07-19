#include <catch.hpp>
#include <fstream>
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
  char* ptr = nullptr;
  gqlite_exec(pHandle,
    "{create: 'movielens',"
      "group: ["
        "{movie: ['title', 'genres']},"
        "{tag: ['user_id', 'movie_id', 'tag']},"
        "{rate: ['user_id', 'movie_id', 'rate']}"
      "]"
    "}",
    gqlite_exec_callback, nullptr, &ptr);
  readCSV("movies.csv", [&pHandle, &ptr](char* buffer) {
    char* movie_id = strtok(buffer, ",");
    if (movie_id == nullptr) return;
    int id = atoi(movie_id);
    char* movie_title = strtok(nullptr, ",");
    std::string title(movie_title);
    char* movie_genres = strtok(nullptr, ",");
    std::string genres(movie_genres);
    std::string data = title + "," + genres;
    char upset[512]= {0};
    sprintf(upset, "{upset: 'movie', vertex: [[%d, {title: '%s', genres: '%s'}]]}", id, title.c_str(), genres.c_str());
    gqlite_exec(pHandle, upset, gqlite_exec_callback, nullptr, &ptr);
  });
}