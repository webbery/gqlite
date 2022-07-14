#include "StorageEngine.h"
#include <iostream>
#include <cassert>
#include <catch.hpp>
#include "Feature/Gist.h"
#include "Type/Vertex.h"

#define GRAPH_NAME  "g1"

#define PRINT_NODE(g, nodeid) \
{\
  std::cout << "==== try to get node: "<<nodeid<<"\n";\
  engine.getNode(g, nodeid, [](const char* key, void* value, int type, void*)->int {\
  std::cout << "    " << key <<": ";\
  switch (type)\
  {\
  case 3:\
    std::cout << (const char*)value << std::endl;\
    break;\
  case 5:\
    std::cout << *(int*)value << std::endl;\
    break;\
  case 6:\
    std::cout << *(unsigned int*)value << std::endl;\
    break;\
  default:\
    break;\
  }\
  return 0;\
});\
}
  

TEST_CASE("basic storage") {
  GStorageEngine engine;
  engine.open("testdb");
  std::cout << "schema: "<< engine.getSchema() << std::endl;
  ClassInfo info;
  info.key_type = 1;
  info.value_type = ClassType::String;
  info.reserved = 0;
  engine.addClass("revert_index", info);
  std::string value("hello gqlite");
  engine.write("revert_index", "key", value.data(), value.size());
  std::string result;
  engine.read("revert_index", "key", result);
  CHECK(result == value);
}

TEST_CASE("range query") {
  GStorageEngine engine;
  engine.open("testdb");
  ClassInfo info;
  info.key_type = 0;
  info.value_type = ClassType::String;
  info.reserved = 0;
  const std::string propname("name");
  engine.addClass(propname, info);
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
  engine.open("testdb");
  ClassInfo info;
  info.key_type = 1;
  info.value_type = ClassType::String;
  info.reserved = 0;
  engine.addClass("index", info);
  std::string value;
  engine.write("index", "key", value.data(), value.size());
  std::string result;
  engine.read("index", "key", result);
  CHECK(result == value);
}