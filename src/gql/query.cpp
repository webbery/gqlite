#include "gql/query.h"
#include "VirtualEngine.h"

namespace query
{
  void get_vertexes(GGraphInstance* pGraph, const std::vector<VertexID>& ids, gqlite_result& results) {
    // results.count = ids.size();
    // gqlite_node* node = nullptr;
    // results.nodes = node;
    // auto itr = ids.begin();
    // for (size_t idx = 0; idx < results.count; ++idx, ++itr) {
    //   node = new gqlite_node;
    //   node->_type = gqlite_node_type_vertex;
    //   node->_vertex = new gqlite_vertex;
    //   const std::string& k = *(itr);
    //   node->_vertex->id = (char*)GMemory::allocate(k.size() + 1);
    //   strncpy(node->_vertex->id, k.c_str(), k.size() + 1);
    //   node->_vertex->properties = nullptr;
    //   node->_vertex->len = 0;
    //   GSinglecton::get<GStorageEngine>()->getNode(pGraph, *itr, [&](const char* s, void*) -> int {
    //     size_t len = strlen(s);
    //     node->_vertex->len = len + 1;
    //     char* c = (char*)malloc(len + 1);
    //     if (!c) return -1;
    //     memcpy(c, s, len);
    //     c[len] = '\0';
    //     node->_vertex->properties = c;
    //     return 0;
    //     });
    // }
  }
  
  void filter_property(gqlite_result& results, gast* properties) {
    if (!properties) return;
    for (size_t idx = 0; idx < results.count; ++idx) {

    }
  }

  void release_vertexes(gqlite_result& results) {
    gqlite_node* node = results.nodes;
    while (node) {
      GMemory::deallocate(node->_vertex->id);
      if (node->_vertex->properties) {
        free(node->_vertex->properties);
      }
      gqlite_node* tmp = node;
      node = node->_next;
      delete tmp;
    }
  }
} // namespace query
