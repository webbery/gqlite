#include <catch.hpp>
#include <thread>
#include <mutex>
#include "VirtualNetwork.h"

std::mutex prod_mut;
std::atomic_bool is_exit(false);

using NodeInfo = std::tuple<GMap::edge_t, std::vector<GMap::node_attr_t>, std::vector<GMap::node_literal_t>>;
using Node = std::map<GMap::node_t, NodeInfo>;
Node nodes = {
  {1, NodeInfo{1, {0}, {0}}},
  {2, NodeInfo{2, {0}, {0}}},
  {3, NodeInfo{3, {0}, {0}}},
  {4, NodeInfo{5, {0}, {0}}},
  {5, NodeInfo{4, {0}, {0}}},
};

void productor(GVirtualNetwork* vn) {
  uint32_t id = 1;
  // while (!is_exit) {
  //   prod_mut.lock();
  //   vn->addNode(id++);
  //   prod_mut.unlock();
  // }
  for (auto& item: nodes) {
    prod_mut.lock();
    vn->addNode(item.first, std::get<1>(item.second), std::get<2>(item.second));
    prod_mut.unlock();
  }
}

class NodeVisitor {
public:
  void operator()(GNode*) {}
};
/**
 * https://people.math.osu.edu/husen.1/teaching/571/random_walks.pdf
 */
TEST_CASE("random walk algorithm") {
  GVirtualNetwork* net = new GVirtualNetwork(10);
  std::thread t(productor, net);
  NodeVisitor visitor;
  net->visit(VisitSelector::RandomWalk, visitor);
  is_exit.store(true);
  t.join();
  // net->release();
  delete net;
  // fmt::print("walk: {}\n", vnames);
}
