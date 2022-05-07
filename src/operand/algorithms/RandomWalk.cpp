#include "operand/algorithms/RandomWalk.h"
#ifdef _PRINT_FORMAT_
#include <fmt/printf.h>
#include <fmt/ranges.h>
#endif

GRandomWalk::GRandomWalk(const GSubGraph& g, double dumping)
: _distribution(0.5, 0.5)
, _dumping(dumping)
, _current(nullptr) {
  size_t start = _distribution(_re) * g.vertex_size();
#ifdef _PRINT_FORMAT_
  fmt::print("start: {}, total: {}\n", start, g.vertex_size());
#endif
  size_t current = 0;
  for (auto itr = g.vertex_begin(), end = g.vertex_end(); itr != end; ++itr, ++current) {
    if (current == start) {
      _current = itr->second;
      break;
    }
  }
}

GVertex* GRandomWalk::next() {
  if (_current) {
    double d = _distribution(_re);
    size_t pos = d * _current->edge_size();
    _current = _current->operator[](pos % _current->edge_size());
#ifdef _PRINT_FORMAT_
    fmt::print("count: {}, pos: {}, distibute: {}\n", _current->edge_size(), pos % _current->edge_size(), d);
#endif
  }
  return _current;
}