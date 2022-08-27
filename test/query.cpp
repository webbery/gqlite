#include "operand/query/HNSW.h"
#include <random>
#include <fmt/printf.h>
#include <fmt/ranges.h>
#include "StorageEngine.h"

const int dim = 128;
const int num_elements = 20;

int main() {
  GStorageEngine* storage = new GStorageEngine();
  GHNSW* instance = new GHNSW(storage, "default", "hnsw");
  std::default_random_engine random;
  std::uniform_real_distribution<float> dis;
  for (int idx = 0; idx < num_elements; ++idx) {
    std::vector<double> vec(dim);
    for (int i = 0; i < dim; ++i) {
      vec[i] = dis(random);
    }
    std::string key = std::to_string(idx + 1);
    //fmt::print("upset: 'vertex_db', vertex: [['{}', @filename: '{}.jpg', feature_name: {}$]\n", key, key, vec);
     instance->add(idx, vec);
  }
  std::vector< std::vector<double> > cond;
  instance->get({ 1 }, cond);
  //assert(cond.size() == 1);
  std::vector<size_t> ids;
  //instance->query(cond[0], 1, ids);
  //assert(ids.size() >= 1);
  delete instance;
}