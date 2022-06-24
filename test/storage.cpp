#include "StorageEngine.h"
#include <iostream>
#include <cassert>
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
  

int main() {
  GStorageEngine engine;
  engine.open("testdb");
  std::cout << "schema: "<< engine.getSchema() << std::endl;
  std::string value("hello gqlite");
  engine.addProp("revert_index", PropertyType::String);
  engine.write("revert_index", "key", value.data(), value.size());
  std::string result;
  engine.read("revert_index", "key", result);
  std::cout<< result<<std::endl;
  return 0;
}