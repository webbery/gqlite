#include "StorageEngine.h"
#include <iostream>
#include <cassert>
#include <catch.hpp>
#include <fstream>

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

TEST_CASE("basic storage api") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  engine.open("testdb", opt);
  std::cout << "schema: "<< engine.getSchema() << std::endl;
  engine.addMap("revert_index", KeyType::Byte);
  std::string value("hello gqlite");
  engine.write("revert_index", "key", value.data(), value.size());
  std::string result;
  engine.read("revert_index", "key", result);
  CHECK(result == value);
}

TEST_CASE("cursor api") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  engine.open("testdb", opt);
  const std::string propname("name");
  engine.addMap(propname, KeyType::Integer);
  int32_t key = 0;
  for (size_t idx = 1; idx < 50; ++idx) {
    std::string value = std::to_string(idx);
    engine.write(propname, idx, value.data(), value.size());
  }
  auto cursor = engine.getCursor(propname);
  mdbx::cursor::move_result result = cursor.to_first(false);
  int idx = 0;
  while (result)
  {
    std::string name((char*)result.value.byte_ptr(), result.value.size());
    CHECK(name == std::to_string(++idx));
    result = cursor.to_next(false);
  }
}

TEST_CASE("empty storage") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  engine.open("testdb", opt);
  engine.addMap("index", KeyType::Byte);
  std::string value;
  engine.write("index", "key", value.data(), value.size());
  std::string result;
  engine.read("index", "key", result);
  CHECK(result == value);
}

TEST_CASE("movielens") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  engine.open("mvlens", opt);
  // create movie map
  engine.addMap("movie", KeyType::Integer);
  readCSV("movies.csv", [&engine](char* buffer) {
    char* movie_id = strtok(buffer, ",");
    if (movie_id == nullptr) return;
    int id = atoi(movie_id);
    char* movie_title = strtok(nullptr, ",");
    std::string title(movie_title);
    char* movie_genres = strtok(nullptr, ",");
    std::string genres(movie_genres);
    std::string data = title + "," + genres;
    engine.write("movie", id, data.data(), data.size());
    });
  std::string a_movie;
  engine.read("movie", 22, a_movie);
  CHECK(a_movie == "Copycat (1995),Crime|Drama|Horror|Mystery|Thriller");

  // create rating map
  engine.addMap("rate", KeyType::Uninitialize);
  readCSV("ratings.csv", [&engine](char* buffer) {
    char* user_id = strtok(buffer, ",");
    if (user_id == nullptr) return;
    uint64_t uid = atoi(user_id);
    char* movie_id = strtok(nullptr, ",");
    uint64_t mid = atoi(movie_id);
    char* movie_rate = strtok(nullptr, ",");
    float rate = atof(movie_rate);
    uint64_t k = (uid << 16 | mid);
    engine.write("rate", k, &rate, sizeof(float));
  });
  uint64_t k = ((uint64_t)517 << 16 | (uint64_t)1721);
  std::string a_rate;
  engine.read("rate", k, a_rate);
  float f_rate = *(float*)a_rate.data();
  CHECK(f_rate == 3.5);

  engine.addMap("tags", KeyType::Integer);
  readCSV("tags.csv", [&engine](char* buffer) {
    char* user_id = strtok(buffer, ",");
    if (user_id == nullptr) return;
    uint64_t uid = atoi(user_id);
    char* movie_id = strtok(nullptr, ",");
    uint64_t mid = atoi(movie_id);
    char* tag = strtok(nullptr, ",");
    uint64_t k = (uid << 16 | mid);
    engine.write("tags", k, tag, strlen(tag));
  });
  std::string a_tag;
  k = ((uint64_t)610 << 16 | (uint64_t)168248);
  engine.read("tags", k, a_tag);
  CHECK(a_tag == "Heroic Bloodshed");
}