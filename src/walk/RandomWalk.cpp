#include "walk/RandomWalk.h"
#ifdef _PRINT_FORMAT_
#include <fmt/printf.h>
#include <fmt/ranges.h>
#endif

GRandomWalk::GRandomWalk(parlay::sequence<GNode*>& visitedNode, double dumping)
: _distribution(0.5, 0.5)
, _dumping(dumping)
, _visited(visitedNode)
{
}

int GRandomWalk::walk(virtual_graph_t& vg, std::function<void(GNode*)> f) {
  
  auto itr = vg._mNodesInstance.begin();
  return 0;
}
// GVertex* GRandomWalk::next() {
//   if (_current) {
//     double d = _distribution(_re);
//     size_t pos = d * _current->edge_size();
//     _current = _current->operator[](pos % _current->edge_size());
// #ifdef _PRINT_FORMAT_
//     fmt::print("count: {}, pos: {}, distibute: {}\n", _current->edge_size(), pos % _current->edge_size(), d);
// #endif
//   }
//   return _current;
// }