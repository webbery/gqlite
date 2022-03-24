#pragma once
#include <string>

class GGraph;
class GStorageEngine;
namespace creation {
  int createGraph(GStorageEngine* storage, const std::string& name);
  int createInvertIndex(GStorageEngine* storage, GGraph* g, const std::string& name);

  std::string generateGraphScript(const std::string& name);
}