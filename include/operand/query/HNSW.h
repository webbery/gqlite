#pragma once
#include <string>
#include <vector>
#include <array>
#include "operand/query/HNSW/hnswlib.h"
#define MAX_LAYER_SIZE  4
#define MAX_NEIGHBOR_SIZE   8
#define MAX_DIMENSION     128
#define MAX_INDEX_COUNT   1*1024*1024

class GHNSW {
public:
  ~GHNSW();
  enum NSWDistance {
    Descart
  };
  static GHNSW* load(NSWDistance dis = Descart);
  static int save();
  static void release(GHNSW* hnsw);
  int add(size_t sid, const std::vector<float>& vec);
  int erase(size_t sid);
  int query(const std::vector<float>& vec, size_t topK, std::vector<size_t>& ids);
  int get(const std::vector<size_t>& ids, std::vector<std::vector<float> >& vecs);

private:
  GHNSW(NSWDistance dis);

  struct InternalVertex {
    std::string _id;
    std::vector<float> _data;
  };

  struct LayerVertex {
    InternalVertex* _vertex;
    std::vector< InternalVertex* > _neighbors;
    LayerVertex* _nextLayerVertex;
  };

  int caculateLayer();

private:
  NSWDistance _distype;
  std::array<LayerVertex*, MAX_LAYER_SIZE> _layers;
  std::array<size_t, MAX_LAYER_SIZE> _sizes;
  hnswlib::L2Space* _pSpace;
  hnswlib::HierarchicalNSW<float>* _instance;
};