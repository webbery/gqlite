#include "base/DOT.h"
#include "fmt/printf.h"
#include <list>
#include <queue>

namespace {
  std::string colorful(int layer) {
    switch (layer)
    {
    case 1:
      return "color=\"#224422\",fontcolor=\"#224422\"";
    case 2:
      return "color=\"#773322\",fontcolor=\"#773322\"";
    case 3:
      return "color=\"#BB2222\",fontcolor=\"#BB2222\"";
    default:
      return "color=black";
    }
  }
}

void print(const virtual_graph_t& graph) {
  fmt::printf("digraph G {\n");
  if (graph.node_size() != 0) {
    auto itr = graph.node_begin();
    std::set<node_t> visited;
    std::list<node_t> queue;
    queue.push_back(itr->first);
    while (queue.size())
    {
      node_t nid = queue.front();
      queue.pop_front();
      if (visited.count(nid)) continue;
      visited.insert(nid);
      auto& node = graph.nodes().at(nid);
      auto& status = std::get<3>(node);
      auto layer = status.layer;
      auto& edges = std::get<0>(node);
      if (edges.size() == 0) {
        fmt::printf("  %lld;\n", (uint64_t)nid);
      }
      std::string edgeInfo;
      for (int8_t level = 0; level < MAX_LAYER_SIZE; ++level) {
        for (auto& eid : edges[level]) {
          auto& edge = graph.edges().at(eid);
          auto& conn = std::get<0>(edge);
          if (conn.first == nid) {
            if (visited.count(conn.second)) continue;
            fmt::printf("  %lld -> %lld [", (uint64_t)nid, (uint64_t)conn.second);
            queue.push_back(conn.second);
          }
          else if (conn.second == nid) {
            if (visited.count(conn.first)) continue;
            fmt::printf("  %lld -> %lld [", (uint64_t)nid, (uint64_t)conn.first);
            queue.push_back(conn.first);
          }
          auto& info = std::get<3>(edge);
          if (info.direction == 0) {
            fmt::printf("dir=none,");
          }
          else if (conn.second == nid) {
            fmt::printf("dir=back,");
          }
          fmt::printf("%s", colorful(level));
          fmt::printf(",label=\"%lld\"", (uint64_t)eid);
          fmt::printf("];\n");
        }
      }
    }
  }
  fmt::printf("}\n");
}