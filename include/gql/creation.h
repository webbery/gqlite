#pragma once
#include <string>

class GGraphInstance;
class GStorageEngine;
namespace creation {
  int createGraph(GStorageEngine* storage, const std::string& name);
  int createInvertIndex(GStorageEngine* storage, GGraphInstance* g, const std::string& name);

  std::string generateGraphScript(const std::string& name);
}