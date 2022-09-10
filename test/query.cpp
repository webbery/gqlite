#include "operand/query/HNSW.h"
#include <random>
#include <fmt/printf.h>
#include <fmt/ranges.h>
#include "StorageEngine.h"
#include "VirtualNetwork.h"
#if defined(_PRINT_FORMAT_)
#include "base/DOT.h"
#endif
#include <catch.hpp>

const int dim = 2;
const int num_elements = 20;

TEST_CASE("add operation") {
  GStorageEngine* storage = new GStorageEngine();
  GVirtualNetwork* net = new GVirtualNetwork(100);
  GHNSW* instance = new GHNSW(net, storage, "default", "hnsw", "hnsw");
  std::default_random_engine random;
  std::uniform_real_distribution<float> dis;
  for (int idx = 0; idx < num_elements; ++idx) {
    std::vector<double> vec(dim);
    for (int i = 0; i < dim; ++i) {
      vec[i] = dis(random);
    }
    std::string key = std::to_string(idx + 1);
    instance->add(static_cast<node_t>(idx), vec, false);
  }
  print(net->_vg);
  instance->save();
  delete storage;
  delete net;
  delete instance;
}

TEST_CASE("query operation") {
  GStorageEngine* storage = new GStorageEngine();
  GVirtualNetwork* net = new GVirtualNetwork(100);
  GHNSW* instance = new GHNSW(net, storage, "default", "hnsw", "hnsw");
  std::vector< std::vector<double> > cond;
  instance->get({ 1 }, cond);
  assert(cond.size() == 1);
  std::vector<uint64_t> ids;
  instance->query(cond[0], 2, ids);
  assert(ids.size() >= 1);
  CHECK_THAT(ids, Catch::Matchers::VectorContains((uint64_t)1));
  delete instance;
  delete net;
  delete storage;
}
