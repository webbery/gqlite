#include "operand/SimilarityOp.h"
#include "SubGraph.h"
#include <fstream>
#include <fmt/printf.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <chrono>
#include <catch.hpp>
#include "operand/algorithms/Hungarian.h"
#include "operand/algorithms/RandomWalk.h"
#include "Graph/BipartiteGraph.h"
#include "Platform.h"
#include <regex>
#include <iostream>
    
std::string working_directory = _WORKING_DIR_ "/test/graphs/";

char* ltrim(char* str) {
  while (isspace(*str)) ++str;
  return str;
}

std::string fisrt_vertex(char* start) {
  char* begin = start;
  while (!isspace(*start) && *start != '-') ++start;
  return std::string(begin, start);
}

std::string last_vertex(char* end) {
  char* offset = end;
  while (!isspace(*end) && *end != '-') --end;
  return std::string(end + 1, offset);
}

#define MAX_READ_SIZE 512
GSubGraph* createGraph(const std::string& dotfile) {
  std::regex bireg(" *([a-zA-Z0-9_]+) *-- *([a-zA-Z0-9_]+) *(\\[[a-zA-Z0-9_]+ *= *([0-9]+)\\]){0,1};");
  std::ifstream fs;
  fs.open(dotfile.c_str());
  GSubGraph* g = new GSubGraph();
  char line[MAX_READ_SIZE] = { 0 };
  while (fs.getline(line, MAX_READ_SIZE)) {
    std::string readed(line);
    std::cmatch cm;
    // printf("read line: %s\n", line);
    if (std::regex_match(readed.c_str(), cm, bireg)) {
      std::string from = cm[1];
      g->addVertex(from);
      std::string to = cm[2];
      g->addVertex(to);
      nlohmann::json attribute;
      std::string sWeight = cm[4];
      // printf("from: %s, to: %s, weight: %s\n", from.c_str(), to.c_str(), sWeight.c_str());
      if (sWeight.size()) {
        int weight = std::stoi(sWeight);
        attribute["weight"] = weight;
      }
      g->addEdge(from, to, GraphEdgeDirection::To, attribute);
    }
    memset(line, 0, MAX_READ_SIZE);
  }
  fs.close();
  return g;
}

void releaseGraph(GSubGraph*g) {
  delete g;
}

TEST_CASE("basic operation", "[member function]") {
  GSubGraph* g1 = createGraph(working_directory + "simple_g.dot");
  GSubGraph* bg = createGraph(working_directory + "bipartite.dot");

  CHECK(g1->isBipartite() == false);
  CHECK(bg->isBipartite() == true);
  

  releaseGraph(g1);
  releaseGraph(bg);
}

TEST_CASE("hungarian algorithm") {
  GSubGraph* wg = createGraph(working_directory + "bipartile_weight.dot");
  GBipartiteGraph bipart = graph_cast<GBipartiteGraph>(*wg);
  GSubGraph* wg2 = createGraph(working_directory + "bipartile_weight_2.dot");
  GBipartiteGraph bipart2 = graph_cast<GBipartiteGraph>(*wg2);
  auto m = bipart.toMatrix("weight");
  //auto m2 = bipart2.toMatrix("weight");
  // fmt::print("hungarian input matrix:\n{}\n", m2);
  double weight = 0;
  HungorianAlgorithm alg;
  alg.solve(m, weight);
  CHECK(weight == 140.0);
  //std::list<std::pair<size_t, size_t>> out;
  //alg.solve(m2, out);
  Eigen::MatrixXd m33(3, 3);
  m33 << 8, 25, 50,
        50, 35, 75,
        22, 48, 150;
  alg.solve(m33, weight);
  CHECK(weight == 107.0);
  Eigen::MatrixXd m88(8, 8);
  m88 << 18, 45, 10, 23, 71, 34, 1, 39,
    5, 135, 75, 81, 73, 49, 16, 98,
    22, 48, 150, 12, 25, 62, 14, 61,
    42, 53, 102, 15, 165, 68, 11, 3,
    9, 48, 150, 9, 77, 62, 14, 61,
    88, 48, 151, 72, 25, 62, 14, 61,
    78, 33, 130, 10, 25, 62, 14, 61,
    18, 67, 110, 12, 25, 62, 14, 61;
  alg.solve(m88, weight);
  CHECK(weight == 155.0);
  BENCHMARK("hungarian algorithm[4x4]") {
    alg.solve(m, weight);
  };
  BENCHMARK("hungarian algorithm[8x8]") {
    alg.solve(m88, weight);
  };
  releaseGraph(wg);
}

/**
 * https://people.math.osu.edu/husen.1/teaching/571/random_walks.pdf
 */
TEST_CASE("random walk algorithm") {
  // enableStackTrace(true);
  GSubGraph* basic = createGraph(working_directory + "random_walk.dot");
  GRandomWalk rw(*basic);
  std::list<std::string> vnames;
  for (size_t cnt = 0; cnt < 5000; ++cnt) {
    GVertex* v = rw.next();
    vnames.push_back(v->id());
  }
  fmt::print("walk: {}\n", vnames);
}

TEST_CASE("distance", "[distance]") {
  GSubGraph* g1 = createGraph(working_directory + "simple_g.dot");
  GSubGraph* g2 = createGraph(working_directory + "g4.dot");
  // CHECK(distance(g1, g2) == 1);
  // BENCHMARK("Graph Edit Distance(A*): N=3") {
  //   distance(g1, g2);
  // };
}
