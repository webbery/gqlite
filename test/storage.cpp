#include "Graph/EntityEdge.h"
#include "Graph/EntityNode.h"
#include "StorageEngine.h"
#include "base/type.h"
#include "gqlite.h"
#include "gutil.h"
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
    if (fs.getline(buff, 512)) {
      if (skip_head) {
        skip_head = false;
        continue;
      }
      cb(buff);
    }
  }
  fs.close();
}

TEST_CASE("basic storage api") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  opt.mode = ReadWriteOption::read_write;
  CHECK(engine.open("storage.db", opt) == ECode_Success);
  engine.addMap("revert_index", KeyType::Byte);
  engine.addMap("revert_index", KeyType::Byte);
  std::string value("hello gqlite");
  CHECK(ECode_Success == engine.write("revert_index", "key", (void*)value.data(), value.size()));
  std::string result;
  engine.read("revert_index", "key", result);
  CHECK(result == value);
}

TEST_CASE("cursor api") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  opt.mode = ReadWriteOption::read_write;
  CHECK(engine.open("storage.db", opt) == ECode_Success);
  const std::string propname("name");
  engine.addMap(propname, KeyType::Integer);
  int32_t key = 0;
  for (size_t idx = 1; idx < 50; ++idx) {
    std::string value = std::to_string(idx);
    engine.write(propname, idx, (void*)value.data(), value.size());
  }
  auto cursor = engine.getMapCursor(propname);
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
  opt.mode = ReadWriteOption::read_write;
  CHECK(engine.open("storage.db", opt) == ECode_Success);
  engine.addMap("index", KeyType::Byte);
  std::string value;
  CHECK(ECode_Success == engine.write("index", "key", (void*)value.data(), value.size()));
  std::string result;
  engine.read("index", "key", result);
  CHECK(result == value);
}

TEST_CASE("movielens") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  opt.mode = ReadWriteOption::read_write;
  engine.open("mvlens.db", opt);
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
    engine.write("movie", id, (void*)data.data(), data.size());
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

TEST_CASE("read_mode") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  opt.mode = ReadWriteOption::read_only;
  CHECK(engine.open("mvlens.db", opt) == ECode_Success);
  uint64_t k = ((uint64_t)517 << 16 | (uint64_t)1721);
  std::string a_rate;
  engine.read("rate", k, a_rate);
  float f_rate = *(float*)a_rate.data();
  CHECK(f_rate == 3.5);
  std::string a_tag;
  k = ((uint64_t)610 << 16 | (uint64_t)168248);
  engine.read("tags", k, a_tag);
  CHECK(a_tag == "Heroic Bloodshed");
}

TEST_CASE("native_storage_api") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  opt.mode = ReadWriteOption::read_write;
  CHECK(engine.open("native_api.db", opt) == ECode_Success);
  engine.addMap("e:follow", KeyType::Uninitialize);
  engine.addMap("follow", KeyType::Uninitialize);
  engine.addMap("user", KeyType::Uninitialize);
  engine.addMap("v:user", KeyType::Uninitialize);
  group_t egid = engine.getGroupID("e:follow");
  group_t gid = engine.getGroupID("v:user");
  CHECK(gid > 0);
  node_t nid = 1;
#define NODE_CNOUNT 4
  GEntityNode* nodes[NODE_CNOUNT];
  for (int i = 0; i < NODE_CNOUNT; ++i) {
    nodes[i] = new GEntityNode(nid++, gid);
    CHECK(upsetVertex(&engine, nodes[i]) == ECode_Success);
  }

  GEntityEdge* edges[4];
  
  edges[0] = new GEntityEdge(egid, nodes[0], nodes[1]);
  upsetEdge(&engine, edges[0]);
  edges[1] = new GEntityEdge(egid, nodes[0], nodes[2]);
  upsetEdge(&engine, edges[1]);
  edges[2] = new GEntityEdge(egid, nodes[1], nodes[2]);
  upsetEdge(&engine, edges[2]);

  auto outbounds = getVertexOutbound(&engine, egid, gid, 2);
  CHECK(outbounds.size() == 1);
  auto neightbors = getVertexNeighbors(&engine, egid, gid, 2);
  CHECK(neightbors.size() == 2);
  auto inbounds = getVertexInbound(&engine, egid, gid, 2);
  CHECK(inbounds.size() == 1);

  edges[3] = new GEntityEdge(egid, nodes[1], nodes[3]);
  upsetEdge(&engine, edges[3]);
  inbounds = getVertexInbound(&engine, egid, gid, 2);
  CHECK(inbounds.size() == 2);

  for (int i = 0; i < 3; ++i) {
    delete edges[i];
  }
  for (int i = 0; i < NODE_CNOUNT; ++i) {
    delete nodes[i];
  }

  
}