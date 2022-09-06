#include <catch.hpp>
#include <thread>
#include <mutex>
#include <chrono>
#include "VirtualNetwork.h"
#include "walk/AStarWalk.h"
#include "walk/BSFWalk.h"
#include "walk/RandomWalk.h"

std::mutex prod_mut;
std::atomic_bool is_exit(false);

using NodeInfo = std::tuple<GMap::edges_t, std::vector<GMap::node_attr_t>, nlohmann::json>;
using Node = std::map<GMap::node_t, NodeInfo>;
Node RomaniaNodes = {
  {(GMap::node_t)1, NodeInfo{{(edge_t)1, (edge_t)7}, {0}, {"address", "Oradea"}} },
  {(GMap::node_t)2, NodeInfo{{(edge_t)1, (edge_t)2}, {0}, {"address", "Zerind"}}},
  {(GMap::node_t)3, NodeInfo{{(edge_t)2, (edge_t)3, (edge_t)8}, {0}, {"address", "Arad"}}},
  {(GMap::node_t)4, NodeInfo{{(edge_t)3, (edge_t)4}, {0}, {"address", "Timisoara"}}},
  {(GMap::node_t)5, NodeInfo{{(edge_t)4, (edge_t)5}, {0}, {"address", "Lugoj"}} },
  {(GMap::node_t)6, NodeInfo{{(edge_t)5, (edge_t)6}, {0}, {"address", "Mehadia"}}},
  {(GMap::node_t)7, NodeInfo{{(edge_t)6, (edge_t)23}, {0}, {"address", "Dobreta"}}},
  {(GMap::node_t)8, NodeInfo{{(edge_t)7, (edge_t)8, (edge_t)11, (edge_t)9}, {0}, {"address", "Sibiu"}}},
  {(GMap::node_t)9, NodeInfo{{(edge_t)9, (edge_t)10, (edge_t)12}, {0}, {"address", "Rimnicu Vilcea"}}},
  {(GMap::node_t)10, NodeInfo{{(edge_t)10, (edge_t)23, (edge_t)13}, {0}, {"address", "Craiova"}}},
  {(GMap::node_t)11, NodeInfo{{(edge_t)11, (edge_t)14}, {0}, {"address", "Fagaras"}}},
  {(GMap::node_t)12, NodeInfo{{(edge_t)12, (edge_t)13, (edge_t)15}, {0}, {"address", "Pitesti"}}},
  {(GMap::node_t)13, NodeInfo{{(edge_t)15, (edge_t)17, (edge_t)14, (edge_t)16}, {0}, {"address", "Bucharest"}}},
  {(GMap::node_t)14, NodeInfo{{(edge_t)16}, {0}, {"address", "Giurgiu"}}},
  {(GMap::node_t)15, NodeInfo{{(edge_t)21}, {0}, {"address", "Neamt"}}},
  {(GMap::node_t)16, NodeInfo{{(edge_t)21, (edge_t)20}, {0}, {"address", "Iasi"}}},
  {(GMap::node_t)17, NodeInfo{{(edge_t)17, (edge_t)18, (edge_t)19}, {0}, {"address", "Urziceni"}}},
  {(GMap::node_t)18, NodeInfo{{(edge_t)20, (edge_t)18}, {0}, {"address", "Vaslui"}}},
  {(GMap::node_t)19, NodeInfo{{(edge_t)19, (edge_t)22}, {0}, {"address", "Hirsova"}}},
  {(GMap::node_t)20, NodeInfo{{(edge_t)22}, {0}, {"address", "Eforie"}}},
};
using EdgeInfo = std::tuple < std::pair<GMap::node_t, GMap::node_t>, int >;
using Edge = std::map<GMap::edge_t, EdgeInfo>;
Edge RomaniaEdges = {
  {(edge_t)1, EdgeInfo{{(GMap::node_t)1, (GMap::node_t)2}, 71}},
  {(edge_t)2, EdgeInfo{{(GMap::node_t)2, (GMap::node_t)3}, 75}},
  {(edge_t)3, EdgeInfo{{(GMap::node_t)3, (GMap::node_t)4}, 118}},
  {(edge_t)4, EdgeInfo{{(GMap::node_t)4, (GMap::node_t)5}, 111}},
  {(edge_t)5, EdgeInfo{{(GMap::node_t)5, (GMap::node_t)6}, 70}},
  {(edge_t)6, EdgeInfo{{(GMap::node_t)6, (GMap::node_t)7}, 75}},
  {(edge_t)7, EdgeInfo{{(GMap::node_t)1, (GMap::node_t)8}, 151}},
  {(edge_t)8, EdgeInfo{{(GMap::node_t)3, (GMap::node_t)8}, 140}},
  {(edge_t)9, EdgeInfo{{(GMap::node_t)8, (GMap::node_t)9}, 80}},
  {(edge_t)10, EdgeInfo{{(GMap::node_t)9, (GMap::node_t)10}, 146}},
  {(edge_t)11, EdgeInfo{{(GMap::node_t)8, (GMap::node_t)11}, 99}},
  {(edge_t)12, EdgeInfo{{(GMap::node_t)9, (GMap::node_t)12}, 97}},
  {(edge_t)13, EdgeInfo{{(GMap::node_t)10, (GMap::node_t)12}, 138}},
  {(edge_t)14, EdgeInfo{{(GMap::node_t)11, (GMap::node_t)13}, 211}},
  {(edge_t)15, EdgeInfo{{(GMap::node_t)12, (GMap::node_t)13}, 101}},
  {(edge_t)16, EdgeInfo{{(GMap::node_t)13, (GMap::node_t)14}, 90}},
  {(edge_t)17, EdgeInfo{{(GMap::node_t)13, (GMap::node_t)17}, 85}},
  {(edge_t)18, EdgeInfo{{(GMap::node_t)17, (GMap::node_t)18}, 142}},
  {(edge_t)19, EdgeInfo{{(GMap::node_t)17, (GMap::node_t)19}, 98}},
  {(edge_t)20, EdgeInfo{{(GMap::node_t)18, (GMap::node_t)16}, 92}},
  {(edge_t)21, EdgeInfo{{(GMap::node_t)16, (GMap::node_t)15}, 87}},
  {(edge_t)22, EdgeInfo{{(GMap::node_t)19, (GMap::node_t)20}, 86}},
  {(edge_t)23, EdgeInfo{{(GMap::node_t)7, (GMap::node_t)10}, 120}},
};

class NodeVisitor {
public:
  void operator()(node_t, const node_info&) {}
};

int cnt = 0;
class NodeLoader {
public:
  NodeLoader(GVirtualNetwork* net) : _net(net) { cnt = 0; }

  bool load() const {
    if (++cnt > maxTimes || is_exit) return false;
    for (auto& item : RomaniaNodes) {
      if (_net->addNode(item.first, std::get<1>(item.second), std::get<2>(item.second))) return false;
    }
    printf("current cnt: %d\n", cnt);
    if (_net->addEdge((edge_t)0, (GMap::node_t)4, (GMap::node_t)5)) return false;
    if (_net->addEdge((edge_t)1, (GMap::node_t)5, (GMap::node_t)4)) return false;
    return true;
  }
private:
  static const int maxTimes = 10;
  GVirtualNetwork* _net;
};

class RomaniaLoader {
public:
  bool load() const {
    if (is_exit) return false;
    return true;
  }

  void loadRomania(GVirtualNetwork* net) {
    int node_cnt = sizeof(RomaniaNodes) / sizeof(Node);
    for (auto& node: RomaniaNodes) {
      net->addNode(node.first, std::get<1>(node.second), std::get<2>(node.second));
    }
    int edge_cnt = sizeof(RomaniaEdges) / sizeof(Edge);
    for (auto& edge : RomaniaEdges) {
      auto from = std::get<0>(edge.second).first;
      auto to = std::get<0>(edge.second).second;
      net->addEdge(edge.first, from, to, { 0 }, { std::get<1>(edge.second) });
      net->addEdge(edge.first, to, from, { 0 }, { std::get<1>(edge.second) });
    }
  }
};

class RomaniaHeuristic : public IAStarHeuristic {
public:
  RomaniaHeuristic(node_t target):IAStarHeuristic(target){
    _distance2Bucharest = {
      {(node_t)3, 366}, {(node_t)13,0}, {(node_t)10, 160}, {(node_t)7, 242}, {(node_t)20, 161},{(node_t)11, 176},{(node_t)14, 77},
      {(node_t)19, 151}, {(node_t)16, 226},{(node_t)5,244},{(node_t)6, 241}, {(node_t)15, 234}, {(node_t)1, 380}, {(node_t)12, 100},
      {(node_t)9, 193}, {(node_t)8, 253}, {(node_t)4, 329}, {(node_t)17, 80}, {(node_t)18, 199}, {(node_t)2, 374}
    };
  }
  double operator()(const node_info& cur, const node_info& node) {
    auto edges_1 = std::get<0>(std::get<1>(cur));
    auto edges_2 = std::get<0>(std::get<1>(node));
    std::vector<edge_t> edges(1);
    auto itr = std::set_intersection(edges_1.begin(), edges_1.end(), edges_2.begin(), edges_2.end(), edges.begin());
    if (itr - edges.begin()) {
      edge_t id = edges[0];
      return (double)std::get<1>(RomaniaEdges[id]);
    }
    return std::numeric_limits<double>::max();
  }

  double h(node_t id) {
    if (_distance2Bucharest.count(id))
      return _distance2Bucharest[id];
    return std::numeric_limits<double>::max();
  }

  std::list<node_t> path() { return _path; }
private:
  std::map<node_t, int> _distance2Bucharest;
};

class AStarSelector: public IAStarWalkSelector< RomaniaHeuristic >{
public:
  AStarSelector(RomaniaHeuristic& h): IAStarWalkSelector(h) {
  }

  void start(node_t from) {
    _pos = from;
  }
};

/**
 * https://people.math.osu.edu/husen.1/teaching/571/random_walks.pdf
 */
TEST_CASE("random walk algorithm") {
  GVirtualNetwork* net = new GVirtualNetwork(10);
  NodeVisitor visitor;
  NodeLoader loader(net);
  GRandomWalkSelector selector("");
  net->visit(selector, visitor, loader);
  net->join();
  delete net;
  assert(cnt == 11);
  // fmt::print("walk: {}\n", vnames);
}

TEST_CASE("bread search first walk algorithm") {
 GVirtualNetwork* net = new GVirtualNetwork(100);
 NodeVisitor visitor;
 NodeLoader loader(net);
 GBFSHeuristic h((node_t)100);
 GBFSSelector selector(h);
 net->visit(selector, visitor, loader);
 is_exit.store(true);
 // std::this_thread::sleep_for(std::chrono::milliseconds(40));
 delete net;
 //assert(cnt == 11);
}

TEST_CASE("A* walk algorithm") {
  printf("Start A*\n");
  is_exit.store(false);
  GVirtualNetwork* net = new GVirtualNetwork(10);
  NodeVisitor visitor;
  RomaniaLoader loader;
  loader.loadRomania(net);
  RomaniaHeuristic h((node_t)13);
  AStarSelector selector(h);
  selector.start((node_t)0);
  net->visit(selector, visitor, loader);
  net->join();
  is_exit.store(true);
  auto& path = h.path();
  for (auto itr = path.begin(); itr != path.end(); ++itr) {
    std::string addr = std::get<2>(RomaniaNodes[*itr])[1];
    printf("%s(%d) -> ", addr.c_str(), *itr);
  }
  printf("\n");
  assert(path.size() == 5);
  auto itr = path.begin();
  assert(*itr++ == (node_t)1);
  assert(*itr++ == (node_t)8);
  assert(*itr++ == (node_t)9);
  assert(*itr++ == (node_t)12);
  assert(*itr++ == (node_t)13);
  delete net;
  // fmt::print("walk: {}\n", vnames);
}