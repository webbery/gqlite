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

TEST_CASE("add operation") {
  const int dim[] = { 8/*, 32, 64, 128*/ };
  const int num_elements[] = { 80/*, 320, 1280, 5120*/ };
  GCoSchedule schedule;
  for (int elm_indx = 0; elm_indx < sizeof(num_elements) / sizeof(int); ++elm_indx) {
    for (int dim_indx = 0; dim_indx < sizeof(dim) / sizeof(int); ++dim_indx) {
      GStorageEngine* storage = new GStorageEngine();
      GVirtualNetwork* net = new GVirtualNetwork(&schedule, 100);
      std::string graph_name("hnsw");
      graph_name += "_" + std::to_string(num_elements[elm_indx]) + "_" + std::to_string(dim[dim_indx]);
      GHNSW* instance = new GHNSW(net, storage, "default", "hnsw", graph_name.c_str());
      std::default_random_engine random;
      std::uniform_real_distribution<float> dis;
      for (int idx = 0; idx < num_elements[elm_indx]; ++idx) {
        std::vector<double> vec(dim[dim_indx]);
        for (int i = 0; i < dim[dim_indx]; ++i) {
          vec[i] = dis(random);
        }
        std::string key = std::to_string(idx + 1);
        instance->add(static_cast<node_t>(idx), vec);
      }
      instance->save();
      delete storage;
      delete net;
      delete instance;
    }
  }
}

TEST_CASE("query operation") {
 const int dim = 2;
 const int num_elements = 20;
 GCoSchedule schedule;
 GStorageEngine* storage = new GStorageEngine();
 GVirtualNetwork* net = new GVirtualNetwork(&schedule, 100);
 GHNSW* instance = new GHNSW(net, storage, "default", "hnsw", "hnsw_80_8");
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

//TEST_CASE("query performance") {
//  GStorageEngine* storage = new GStorageEngine();
//  GVirtualNetwork* net = new GVirtualNetwork(100);
//  GHNSW* instance = new GHNSW(net, storage, "default", "hnsw", "hnsw");
//  std::vector< std::vector<double> > cond;
//  instance->get({ 1 }, cond);
//  assert(cond.size() == 1);
//  std::vector<uint64_t> ids;
//  instance->query(cond[0], 2, ids);
//  assert(ids.size() >= 1);
//  CHECK_THAT(ids, Catch::Matchers::VectorContains((uint64_t)1));
//  delete instance;
//  delete net;
//  delete storage;
//}

TEST_CASE("delete operation") {

}