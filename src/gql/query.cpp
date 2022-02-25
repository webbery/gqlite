#include "gql/query.h"
#include "Memory.h"
#include "VirtualEngine.h"
#include "Singlecton.h"

namespace query
{
  void get_vertexes(GGraph* pGraph, const std::vector<VertexID>& ids, gqlite_result& results) {
    results.count = ids.size();
    if (results.count) {
        results.nodes = new gqlite_node[results.count];
        for (size_t idx = 0; idx < results.count; ++idx) {
          const std::string& k = ids[idx];
          results.nodes[idx].id = (char*)GMemory::allocate(k.size() + 1);
          strncpy(results.nodes[idx].id, k.c_str(), k.size() + 1);
          results.nodes[idx].len = 0;
          results.nodes[idx].properties = nullptr;
          GSinglecton::get<GStorageEngine>()->getNode(pGraph, ids[idx], [&](const char* s, void*) -> int {
              size_t len = strlen(s);
              results.nodes[idx].len = len + 1;
              char* c = (char*)malloc(len + 1);
              if (!c) return -1;
              memcpy(c, s, len);
              c[len] = '\0';
              results.nodes[idx].properties = c;
              return 0;
          });
        }
    } else {
        results.nodes = nullptr;
    }
  }
  
  void get_vertexes(GGraph* pGraph, const std::set<VertexID>& ids, gqlite_result& results) {
    results.count = ids.size();
    if (results.count) {
      results.nodes = new gqlite_node[results.count];
      auto itr = ids.begin();
      for (size_t idx = 0; idx < results.count; ++idx, ++itr) {
        const std::string& k = *(itr);
        results.nodes[idx].id = (char*)GMemory::allocate(k.size() + 1);
        strncpy(results.nodes[idx].id, k.c_str(), k.size() + 1);
        results.nodes[idx].len = 0;
        results.nodes[idx].properties = nullptr;
        GSinglecton::get<GStorageEngine>()->getNode(pGraph, *(itr), [&](const char* s, void*) -> int {
            size_t len = strlen(s);
            results.nodes[idx].len = len + 1;
            char* c = (char*)malloc(len + 1);
            if (!c) return -1;
            memcpy(c, s, len);
            c[len] = '\0';
            results.nodes[idx].properties = c;
            return 0;
        });
      }
    } else {
      results.nodes = nullptr;
    }
  }
  void filter_property(gqlite_result& results, gql_node* properties) {
    if (!properties) return;
    for (size_t idx = 0; idx < results.count; ++idx) {

    }
  }

  void release_vertexes(gqlite_result& results) {
    if (results.count) {
        for (size_t idx = 0; idx < results.count; ++idx) {
          GMemory::deallocate(results.nodes[idx].id);
          if (results.nodes[idx].properties) {
            free(results.nodes[idx].properties);
          }
        }
        delete[] results.nodes;
    }
  }
} // namespace query
