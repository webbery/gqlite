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

class GStorageEngine;
class GHNSW {
public:
  ~GHNSW();
  enum NSWDistance {
    Descart
  };
  /**
   * hnsw is saved by storage, which is a k-v database.
   * @param index_name an reverted index which key is point id, and value is near point's id
   * @param graph a filename that save to disk
   */
  GHNSW(GStorageEngine* storage, const char* index_name, const char* graph = nullptr);
  void release(GHNSW* hnsw);
  int add(uint64_t sid, const std::vector<double>& vec);
  int erase(size_t sid);
  int query(const std::vector<double>& vec, size_t topK, std::vector<uint64_t>& ids);
  int get(const std::vector<uint64_t>& ids, std::vector<std::vector<double> >& vecs);

private:
  bool initHNSW(const std::string& key, size_t dim);
  void load();

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
  /**
   * an index name in graph database
   */
  std::string _index;
  GStorageEngine* _storage;
  //HNSW* _activeHNSW;
};