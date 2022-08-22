#include <catch.hpp>
#include <thread>
#include <mutex>
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

class NodeLoader {
public:
  NodeLoader(GVirtualNetwork* net): _net(net) {}

  void load() const {
    prod_mut.lock();
    for (auto& item : nodes) {
      _net->addNode(item.first, std::get<1>(item.second), std::get<2>(item.second));
    }
    _net->addEdge(0, 4, 5);
    _net->addEdge(1, 5, 4);
    prod_mut.unlock();
  }
private:
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
  is_exit.store(true);
  net->release();
  delete net;
  // fmt::print("walk: {}\n", vnames);
}

TEST_CASE("bread search first walk algorithm") {
  GVirtualNetwork* net = new GVirtualNetwork(100);
  NodeVisitor visitor;
  NodeLoader loader(net);
  net->visit(VisitSelector::BreadSearchFirst, visitor, loader);
  is_exit.store(true);
  net->release();
  delete net;
}