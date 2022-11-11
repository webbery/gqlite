#include <fstream>
#include <fmt/printf.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <chrono>
#include <catch.hpp>
#include <regex>
#include <iostream>
#include "operand/algorithms/Hungarian.h"
#include "base/system/Platform.h"
#include "operand/analysis/DegreeCentrality.h"
    
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

TEST_CASE("hungarian algorithm") {
  double weight = 0;
  HungorianAlgorithm alg;
  //std::list<std::pair<size_t, size_t>> out;
  //alg.solve(m2, out);
  Eigen::MatrixXd m33(3, 3);
  m33 << 8, 25, 50,
        50, 35, 75,
        22, 48, 150;
  alg.solve(m33, weight);
  CHECK(weight == 107.0);

  Eigen::MatrixXd m66(6, 6);
  m66 << 62, 75, 80, 93, 95, 97,
    75, 80, 82, 85, 71, 97,
    80, 75, 81, 98, 90, 97,
    78, 82, 84, 80, 50, 98,
    90, 85, 85, 80, 85, 99,
    65, 75, 80, 75, 68, 96;
  alg.solve(m66, weight);
  CHECK(weight == 443.0);

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

  BENCHMARK("hungarian algorithm[8x8]") {
    alg.solve(m88, weight);
  };
}
