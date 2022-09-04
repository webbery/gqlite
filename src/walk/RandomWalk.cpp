#include "walk/RandomWalk.h"
#ifdef _PRINT_FORMAT_
#include <fmt/printf.h>
#include <fmt/ranges.h>
#endif

GRandomWalkSelector::GRandomWalkSelector(const std::string& prop, double dumping)
: _distribution(0.5, 0.5)
, _dumping(dumping)
{
}

void GRandomWalkSelector::stand(virtual_graph_t& vg)
{

}

int GRandomWalkSelector::walk(virtual_graph_t& vg, std::function<void(node_t, const node_info&)> f) {
  // auto itr = vg._mNodesInstance.begin();
  return WalkResult::WR_Visited|WalkResult::WR_Preload;
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