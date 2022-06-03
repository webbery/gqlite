#include "StorageEngine.h"
#include <iostream>
#include <cassert>
#include "Feature/Gist.h"
#include "Type/Vertex.h"
#include "base/VertexVisitor.h"

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
  engine.create("testdb");
  auto graphs = engine.getGraphs();

  std::cout << "show graphs:" << std::endl;
  for (auto name : graphs) {
    std::cout << name << std::endl;
  }
  assert(engine.openGraph(GRAPH_NAME) == 0);
  auto* pGraph = engine.getGraph(GRAPH_NAME);
  assert(pGraph != nullptr);
  //gast* root = loadast("storage.ast");
  //if (root) {
  //  ASTVertexUpdateVisitor visitor;
  //  traverse(root, &visitor);
  //}
  
  engine.makeDirection(pGraph, "id1", "father", "mother", "wife");
  engine.dropNode(pGraph, "mother");
  engine.finishUpdate(pGraph);
  std::vector<VertexID> ids = engine.getNodes(pGraph);
  std::cout<<"size: "<<ids.size()<<std::endl;
  //assert(ids.size() > 0);
  PRINT_NODE(pGraph, "mother");
  PRINT_NODE(pGraph, "father");

  GVertexProptertyFeature* pBTreeIndex = new GBTreeIndex("create_time");
  engine.registGraphFeature(pGraph, pBTreeIndex);
  
  engine.finishUpdate(pGraph);
  PRINT_NODE(pGraph, "photo_father");
  
  //ids = engine.getNodes(pGraph, "create_time",
  //  []() {},
  //  1, 4);
  std::cout << "size: " << ids.size() << std::endl;
  engine.closeGraph(pGraph);
  return 0;
}