#pragma once
#include <vector>
#include <set>
#include "gqlite.h"
#include "Graph.h"
#include "Memory.h"
#include "base/ast.h"
#include "base/list.h"

namespace query
{
  template<typename Container>
  void get_vertexes(GGraph* pGraph, const Container& ids, gqlite_result& results) {
    results.count = ids.size();
    gqlite_node* node = nullptr;
    gqlite_node* root = node;
    auto itr = ids.begin();
    for (size_t idx = 0; idx < results.count; ++idx, ++itr) {
      node = new gqlite_node;
      node->_type = gqlite_node_type_vertex;
      node->_vertex = new gqlite_vertex;
      const std::string& k = *(itr);
      node->_vertex->id = (char*)GMemory::allocate(k.size() + 1);
      strncpy(node->_vertex->id, k.c_str(), k.size() + 1);
      node->_vertex->properties = nullptr;
      node->_vertex->len = 0;
      GSinglecton::get<GStorageEngine>()->getNode(pGraph, *itr, [&](const char* s, void*) -> int {
        size_t len = strlen(s);
        node->_vertex->len = len + 1;
        char* c = (char*)malloc(len + 1);
        if (!c) return -1;
        memcpy(c, s, len);
        c[len] = '\0';
        node->_vertex->properties = c;
        return 0;
        });
      node->_next = nullptr;
      root = list_join(root, node);
    }
    results.nodes = root;
  }
  void filter_property(gqlite_result& results, gast* properties);
  void release_vertexes(gqlite_result& results);
} // namespace query
