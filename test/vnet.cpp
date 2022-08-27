#include <catch.hpp>
#include <thread>
#include <mutex>
#include <chrono>
#include "VirtualNetwork.h"

std::mutex prod_mut;
std::atomic_bool is_exit(false);

using NodeInfo = std::tuple<GMap::edge_t, std::vector<GMap::node_attr_t>, nlohmann::json>;
using Node = std::map<GMap::node_t, NodeInfo>;
Node nodes = {
  {1, NodeInfo{{}, {0}, {"name", "1"}} },
  {2, NodeInfo{{}, {0}, {"name", "2"}}},
  {3, NodeInfo{{}, {0}, {"name", "3"}}},
  {4, NodeInfo{0, {0}, {"name", "4"}}},
  {5, NodeInfo{1, {0}, {"name", "5"}}},
};

class NodeVisitor {
public:
  void operator()(GNode*) {}
};

int cnt = 0;
class NodeLoader {
public:
  NodeLoader(GVirtualNetwork* net) : _net(net) { cnt = 0; }

  bool load() const {
    if (++cnt > maxTimes || is_exit) return false;
    for (auto& item : nodes) {
      if (_net->addNode(item.first, std::get<1>(item.second), std::get<2>(item.second))) return false;
    }
    printf("current cnt: %d\n", cnt);
    if (_net->addEdge(0, 4, 5)) return false;
    if (_net->addEdge(1, 5, 4)) return false;
    return true;
  }
private:
  static const int maxTimes = 10;
  GVirtualNetwork* _net;
};
/**
 * https://people.math.osu.edu/husen.1/teaching/571/random_walks.pdf
 */
TEST_CASE("random walk algorithm") {
  GVirtualNetwork* net = new GVirtualNetwork(10);
  NodeVisitor visitor;
  NodeLoader loader(net);
  net->visit(VisitSelector::RandomWalk, visitor, loader);
  net->join();
  delete net;
  assert(cnt == 11);
  // fmt::print("walk: {}\n", vnames);
}

TEST_CASE("bread search first walk algorithm") {
  GVirtualNetwork* net = new GVirtualNetwork(100);
  NodeVisitor visitor;
  NodeLoader loader(net);
  net->visit(VisitSelector::BreadSearchFirst, visitor, loader);
  is_exit.store(true);
  // std::this_thread::sleep_for(std::chrono::milliseconds(40));
  delete net;
  //assert(cnt == 11);
}