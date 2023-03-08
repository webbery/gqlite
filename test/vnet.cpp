#include <catch.hpp>
#include <chrono>
#include "ScanObserver.h"
#include "StorageEngine.h"
#include "VirtualNetwork.h"
#include "walk/AStarWalk.h"
#include "walk/BSFWalk.h"
#include "walk/RandomWalk.h"
#include "walk/BidirectionWalk.h"
#include "Loader.h"
#include "plan/ScanPlan.h"

std::atomic_bool is_exit(false);

using net = GMap<uint64_t, uint64_t>;
using NodeInfo = std::tuple<net::edges_t, std::vector<net::node_attr_t>, nlohmann::json>;
using Node = std::map<net::node_t, NodeInfo>;
Node RomaniaNodes = {
  {(net::node_t)1, NodeInfo{{(edge_t)1, (edge_t)7}, {0}, {"address", "Oradea"}} },
  {(net::node_t)2, NodeInfo{{(edge_t)1, (edge_t)2}, {0}, {"address", "Zerind"}}},
  {(net::node_t)3, NodeInfo{{(edge_t)2, (edge_t)3, (edge_t)8}, {0}, {"address", "Arad"}}},
  {(net::node_t)4, NodeInfo{{(edge_t)3, (edge_t)4}, {0}, {"address", "Timisoara"}}},
  {(net::node_t)5, NodeInfo{{(edge_t)4, (edge_t)5}, {0}, {"address", "Lugoj"}} },
  {(net::node_t)6, NodeInfo{{(edge_t)5, (edge_t)6}, {0}, {"address", "Mehadia"}}},
  {(net::node_t)7, NodeInfo{{(edge_t)6, (edge_t)23}, {0}, {"address", "Dobreta"}}},
  {(net::node_t)8, NodeInfo{{(edge_t)7, (edge_t)8, (edge_t)11, (edge_t)9}, {0}, {"address", "Sibiu"}}},
  {(net::node_t)9, NodeInfo{{(edge_t)9, (edge_t)10, (edge_t)12}, {0}, {"address", "Rimnicu Vilcea"}}},
  {(net::node_t)10, NodeInfo{{(edge_t)10, (edge_t)23, (edge_t)13}, {0}, {"address", "Craiova"}}},
  {(net::node_t)11, NodeInfo{{(edge_t)11, (edge_t)14}, {0}, {"address", "Fagaras"}}},
  {(net::node_t)12, NodeInfo{{(edge_t)12, (edge_t)13, (edge_t)15}, {0}, {"address", "Pitesti"}}},
  {(net::node_t)13, NodeInfo{{(edge_t)15, (edge_t)17, (edge_t)14, (edge_t)16}, {0}, {"address", "Bucharest"}}},
  {(net::node_t)14, NodeInfo{{(edge_t)16}, {0}, {"address", "Giurgiu"}}},
  {(net::node_t)15, NodeInfo{{(edge_t)21}, {0}, {"address", "Neamt"}}},
  {(net::node_t)16, NodeInfo{{(edge_t)21, (edge_t)20}, {0}, {"address", "Iasi"}}},
  {(net::node_t)17, NodeInfo{{(edge_t)17, (edge_t)18, (edge_t)19}, {0}, {"address", "Urziceni"}}},
  {(net::node_t)18, NodeInfo{{(edge_t)20, (edge_t)18}, {0}, {"address", "Vaslui"}}},
  {(net::node_t)19, NodeInfo{{(edge_t)19, (edge_t)22}, {0}, {"address", "Hirsova"}}},
  {(net::node_t)20, NodeInfo{{(edge_t)22}, {0}, {"address", "Eforie"}}},
};
using EdgeInfo = std::tuple < std::pair<net::node_t, net::node_t>, int >;
using Edge = std::map<net::edge_t, EdgeInfo>;
Edge RomaniaEdges = {
  {(edge_t)1, EdgeInfo{{(net::node_t)1, (net::node_t)2}, 71}},
  {(edge_t)2, EdgeInfo{{(net::node_t)2, (net::node_t)3}, 75}},
  {(edge_t)3, EdgeInfo{{(net::node_t)3, (net::node_t)4}, 118}},
  {(edge_t)4, EdgeInfo{{(net::node_t)4, (net::node_t)5}, 111}},
  {(edge_t)5, EdgeInfo{{(net::node_t)5, (net::node_t)6}, 70}},
  {(edge_t)6, EdgeInfo{{(net::node_t)6, (net::node_t)7}, 75}},
  {(edge_t)7, EdgeInfo{{(net::node_t)1, (net::node_t)8}, 151}},
  {(edge_t)8, EdgeInfo{{(net::node_t)3, (net::node_t)8}, 140}},
  {(edge_t)9, EdgeInfo{{(net::node_t)8, (net::node_t)9}, 80}},
  {(edge_t)10, EdgeInfo{{(net::node_t)9, (net::node_t)10}, 146}},
  {(edge_t)11, EdgeInfo{{(net::node_t)8, (net::node_t)11}, 99}},
  {(edge_t)12, EdgeInfo{{(net::node_t)9, (net::node_t)12}, 97}},
  {(edge_t)13, EdgeInfo{{(net::node_t)10, (net::node_t)12}, 138}},
  {(edge_t)14, EdgeInfo{{(net::node_t)11, (net::node_t)13}, 211}},
  {(edge_t)15, EdgeInfo{{(net::node_t)12, (net::node_t)13}, 101}},
  {(edge_t)16, EdgeInfo{{(net::node_t)13, (net::node_t)14}, 90}},
  {(edge_t)17, EdgeInfo{{(net::node_t)13, (net::node_t)17}, 85}},
  {(edge_t)18, EdgeInfo{{(net::node_t)17, (net::node_t)18}, 142}},
  {(edge_t)19, EdgeInfo{{(net::node_t)17, (net::node_t)19}, 98}},
  {(edge_t)20, EdgeInfo{{(net::node_t)18, (net::node_t)16}, 92}},
  {(edge_t)21, EdgeInfo{{(net::node_t)16, (net::node_t)15}, 87}},
  {(edge_t)22, EdgeInfo{{(net::node_t)19, (net::node_t)20}, 86}},
  {(edge_t)23, EdgeInfo{{(net::node_t)7, (net::node_t)10}, 120}},
};

int cnt = 0;
class NodeLoader {
public:
  NodeLoader(GVirtualNetwork* net) : _net(net) { cnt = 0; }

  bool load() const {
    if (++cnt > maxTimes) return false;
    for (auto& item : RomaniaNodes) {
      if (_net->addNode(item.first, std::get<1>(item.second), std::get<2>(item.second))) return false;
    }
#if defined(GQLITE_ENABLE_PRINT)
    printf("current cnt: %d\n", cnt);
#endif
    _net->addEdge((edge_t)0, (net::node_t)4, (net::node_t)5);
    _net->addEdge((edge_t)1, (net::node_t)5, (net::node_t)4);
    return true;
  }
private:
  static const int maxTimes = 10;
  GVirtualNetwork* _net;
};

class RomaniaLoader {
public:
  bool load() const {
    return false;
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
    auto edges_1 = std::get<0>(std::get<1>(cur))[0];
    auto edges_2 = std::get<0>(std::get<1>(node))[0];
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

class AStarSelector: public IAStarWalkSelector<virtual_graph_t, RomaniaHeuristic >{
public:
  AStarSelector(RomaniaHeuristic& h): IAStarWalkSelector<virtual_graph_t, RomaniaHeuristic >(h) {
  }

  void start(virtual_graph_t::node_t from) {
    IAStarWalkSelector<virtual_graph_t, RomaniaHeuristic >::_pos = from;
  }
};

/**
 * https://people.math.osu.edu/husen.1/teaching/571/random_walks.pdf
 */
TEST_CASE("random walk algorithm") {
  GCoSchedule schedule;
  GVirtualNetwork* net = new GVirtualNetwork(&schedule, 10);
  std::function<void(uint64_t, const node_info&)> visitor = [](node_t, const node_info&) {};
  NodeLoader loader(net);
  GRandomWalkSelector selector("");
  net->visit(selector, visitor, loader);
  schedule.run();
  delete net;
  CHECK(cnt == 11);
  // fmt::print("walk: {}\n", vnames);
}

 TEST_CASE("bread search first walk algorithm") {
   GCoSchedule schedule;
   GVirtualNetwork* net = new GVirtualNetwork(&schedule, 100);
   auto visitor = [](node_t, const node_info&) {};
   NodeLoader loader(net);
   GBFSHeuristic h((node_t)100);
   GBFSSelector<virtual_graph_t> selector(h);
   net->visit(selector, visitor, loader);
   schedule.run();
   is_exit.store(true);
   // std::this_thread::sleep_for(std::chrono::milliseconds(40));
   delete net;
   //assert(cnt == 11);
 }

TEST_CASE("bread search first walk algorithm with native loader") {
  //GCoSchedule schedule;
  //GVirtualNetwork* net = new GVirtualNetwork(&schedule, 100);
  //NodeVisitor visitor;
  //GStorageEngine engine;
  //StoreOption opt;
  //opt.compress = 0;
  //opt.mode = ReadWriteOption::read_only;
  //assert(engine.open("basketballplayer",  opt) == ECode_Success);
  //
  //std::map<std::string, GVirtualNetwork*> networds = {{"follow", net}};
  //GScanPlan plan(networds, &engine, nullptr, "follow");

  //GLoader loader;
  //GScanObserver* observer = new GScanObserver(&loader);
  //plan.addObserver(observer);
  //
  //GBFSHeuristic h((node_t)100);
  //GBFSSelector<virtual_graph_t> selector(h);
  //net->visit(selector, visitor, loader);
  //engine.close();
  //delete net;
}

 TEST_CASE("A* walk algorithm") {
   GCoSchedule schedule;
   GVirtualNetwork* net = new GVirtualNetwork(&schedule, 10);
   auto visitor = [](node_t, const node_info&) {};
   RomaniaLoader loader;
   loader.loadRomania(net);
   RomaniaHeuristic h((node_t)13);
   AStarSelector selector(h);
   selector.start((node_t)0);
   net->visit(selector, visitor, loader);
   schedule.run();
   std::vector<uint64_t> path;
   for (auto nid : h.path()) {
     path.push_back((uint64_t)nid);
   }
   CHECK(path.size() == 5);
   //for (auto itr = path.begin(); itr != path.end(); ++itr) {
   //  std::string addr = std::get<2>(RomaniaNodes[*itr])[1];
   //  printf("%s(%ld) -> ", addr.c_str(), *itr);
   //}
   std::vector<uint64_t> ln = { 1, 8, 9, 12, 13 };
   CHECK_THAT(path, Catch::Matchers::Equals(ln));
   //printf("\n");
   auto itr = path.begin();
   CHECK(*itr++ == 1);
   CHECK(*itr++ == 8);
   CHECK(*itr++ == 9);
   CHECK(*itr++ == 12);
   CHECK(*itr++ == 13);
   delete net;
 }

//TEST_CASE("performance") {
//  GVirtualNetwork* net = new GVirtualNetwork(10);
//  NodeVisitor visitor;
//  RomaniaLoader loader;
//  loader.loadRomania(net);
//  int index = 0;
//  BENCHMARK("A* Search Benchmark") {
//    is_exit.store(false);
//    RomaniaHeuristic h((node_t)14);
//    AStarSelector selector(h);
//    selector.start((node_t)1);
//    net->visit(selector, visitor, loader);
//    net->join();
//    is_exit.store(true);
//    ++index;
//  };
//  delete net;
//}