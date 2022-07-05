#include <catch.hpp>
#include <thread>
#include <mutex>
#include "VirtualNetwork.h"

std::mutex prod_mut;
std::atomic_bool is_exit(false);

void productor(GVirtualNetwork* vn) {
  uint32_t id = 1;
  while (!is_exit) {
    prod_mut.lock();
    vn->addNode(id++);
    prod_mut.unlock();
  }
}

/**
 * https://people.math.osu.edu/husen.1/teaching/571/random_walks.pdf
 */
TEST_CASE("random walk algorithm") {
  GVirtualNetwork* net = new GVirtualNetwork(10);
  std::thread t(productor, net);
  net->visit(VisitSelector::RandomWalk, [](GNode*) {});
  is_exit.store(true);
  t.join();
  net->release();
  delete net;
  // fmt::print("walk: {}\n", vnames);
}
