#pragma once
#include <string>
#include <vector>
#include <array>
#include <map>
//#include "operand/query/HNSW/hnswlib.h"
#define MAX_LAYER_SIZE  4
#define MAX_NEIGHBOR_SIZE   8
#define MAX_DIMENSION     128
#define MAX_INDEX_COUNT   1*1024*1024

class GHNSWManager {
public:
  ~GHNSWManager();
  enum NSWDistance {
    Descart
  };
  GHNSWManager(const char* graph);
  /**
   * @param index_file an index file with [graph]_[dim]_[index].indx
   */
  int load(const std::string& index_file);
  int save(const std::string& index_file);
  int flush();
  static void release(GHNSWManager* hnsw);
  int add(size_t sid, const std::vector<float>& vec);
  int erase(size_t sid);
  int query(const std::vector<float>& vec, size_t topK, std::vector<size_t>& ids);
  int get(const std::vector<size_t>& ids, std::vector<std::vector<float> >& vecs);

private:
  bool initHNSW(const std::string& key, size_t dim);

  struct InternalVertex {
    std::string _id;
    std::vector<float> _data;
  };

  struct LayerVertex {
    InternalVertex* _vertex;
    std::vector< InternalVertex* > _neighbors;
    LayerVertex* _nextLayerVertex;
  };

  //struct HNSW {
  //  hnswlib::L2Space* _pSpace;
  //  hnswlib::HierarchicalNSW<float>* _instance;
  //};

private:
  NSWDistance _distype;
  std::array<LayerVertex*, MAX_LAYER_SIZE> _layers;
  std::array<size_t, MAX_LAYER_SIZE> _sizes;
  //std::map<std::string, HNSW*> _mHNSWs;
  std::string _activeGraph;
  std::string _activeFilename;
  //HNSW* _activeHNSW;
};