#pragma once
#include <string>

class GGraph;
namespace creation {
  int createGraph(const std::string& name);
  int createInvertIndex(GGraph* g, const std::string& name);

  std::string generateGraphScript(const std::string& name);
}