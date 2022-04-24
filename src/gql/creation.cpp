#include "gql/creation.h"
#include "VirtualEngine.h"
#include "Feature/InvertIndex.h"
#include "Error.h"
#include "StorageEngine.h"

namespace creation {
  int createGraph(GStorageEngine* storage, const std::string& name) {
    return storage->openGraph(name.c_str());
  }

  int createInvertIndex(GStorageEngine* storage, GGraphInstance* g, const std::string& name) {
    createGraph(storage, name);
    GIndex* gi = new GIndex(name.c_str());
    storage->registGraphFeature(g, gi);
    return ECode_Success;
  }

  std::string generateGraphScript(const std::string& name) {
    std::string line = "{create: '" + name + "'";
    // GGraph* g = GSinglecton::get<GStorageEngine>()->getGraph(name.c_str());
    // if (!g) {
    //     line += "}\n";
    //     return line;
    // }
    // auto property = g->property();
    // // add index
    // for (std::string& index: property._indexes) {
    //   line += ", index: ['" + index + "',";
    // }
    // if (property._indexes.size()) {
    //   line.pop_back();
    //   line += "]";
    // }
    line += "}\n";
    return line;
  }
}// namespace creation