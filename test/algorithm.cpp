#include "operand/SimilarityOp.h"
#include "SubGraph.h"
#include <fstream>
#include <fmt/printf.h>
#include <chrono>
#include <fmt/color.h>
#include <catch.hpp>
#include "operand/algorithms/Hungarian.h"
#include "Graph/BipartiteGraph.h"
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

TEST_CASE("algorithm", "[hungarian]") {
  GSubGraph* wg = createGraph(working_directory + "bipartile_weight.dot");
  GBipartiteGraph bipart = graph_cast<GBipartiteGraph>(*wg);
  auto m = bipart.toMatrix("weight");
  fmt::print("hungarian input matrix:\n{}\n", m);
  HungorianAlgorithm alg;
  Eigen::MatrixXd out;
  // BENCHMARK("hungarian algorithm[4x4]") {
    alg.solve(m, out);
  // };
  releaseGraph(wg);
}

TEST_CASE("distance", "[distance]") {
  GSubGraph* g1 = createGraph(working_directory + "simple_g.dot");
  GSubGraph* g2 = createGraph(working_directory + "g4.dot");
  // CHECK(distance(g1, g2) == 1);
  // BENCHMARK("Graph Edit Distance(A*): N=3") {
  //   distance(g1, g2);
  // };
}
// int main()
// {
//   int ret = 0;
//   GSubGraph* g1 = createGraph(working_directory + "simple_g.dot");
//   GSubGraph* gs = createGraph(working_directory + "simple_g.dot");
//   GSubGraph* g2 = createGraph(working_directory + "g4.dot");
//   GSubGraph* g3 = createGraph(working_directory + "simple_g_2.dot");
//   GSubGraph* g4 = createGraph(working_directory + "g4_2.dot");
//   GSubGraph* g5 = createGraph(working_directory + "simple_g_3.dot");
//   GSubGraph* bg = createGraph(working_directory + "bipartite.dot");
//   auto m1 = bg->toMatrix();
//   fmt::print("simple_g:\n{}\n", m1);
//   ASSERT_EQ(*g1 == *gs, true);
//   ASSERT_EQ(g1->isBipartite(), false);
//   ASSERT_EQ(g4->isBipartite(), false);
//   ASSERT_EQ(bg->isBipartite(), true);
//   //ASSERT_EQ(distance(g1, g2) == 1, true);
//   releaseGraph(g1);
//   releaseGraph(g2);
//   releaseGraph(g3);
//   releaseGraph(g4);
//   releaseGraph(g5);
//   return ret;
// }