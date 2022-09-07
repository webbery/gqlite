#pragma once

template <typename HeuristicWalk>
class GBidirectionWalk {
  enum {Start, End, Max};
public:
  
  void from() {}

  void to() {}

  int walk(virtual_graph_t& vg, std::function<void(node_t, const node_info&)>) {
    return 0;
  }
private:
  HeuristicWalk _walk[Max];
};