#pragma once
#include <queue>
#include <stdint.h>
#include <string>
#include <vector>
#include <array>
#include <list>
#include <map>
#include <random>
#include "walk/AStarWalk.h"

#define MAX_LAYER_SIZE  4
#define MAX_NEIGHBOR_SIZE   8
#define MAX_DIMENSION     128
#define MAX_INDEX_COUNT   1*1024*1024

class GVirtualNetwork;
class GStorageEngine;

namespace gql {
  class GHNSWHeuristic : public IAStarHeuristic {
  public:
    GHNSWHeuristic() :IAStarHeuristic((node_t)0) {}
    double operator()(const node_info& cur, const node_info& node) {
      return 0;
    }

    void update(node_t target) {
      _target = target;
    }
  };
  class GHNSWAStarSelector : public IAStarWalkSelector< GHNSWHeuristic > {
  public:
    GHNSWAStarSelector(GHNSWHeuristic& h) : IAStarWalkSelector(h) {}

    void start(node_t id) { _pos = id; }
  };
}
class GHNSW {
public:
  ~GHNSW();
  enum NSWDistance {
    Descart
  };
  /**
   * hnsw is saved by storage, which is a k-v database.
   * @param index_name an reverted index which key is point id, and value is near point's id
   * @param prop vector data that save to.
   * @param graph a filename that save to disk
   */
  GHNSW(GVirtualNetwork* network, GStorageEngine* store, const char* index_name, const char* prop, const char* graph = nullptr);

  /**
   * @brief init some feature that hnsw algorithm needs.
   * @param M the number of established connections
   * @param ef_construction the size of the dynamic candidate list
   */
  void init(size_t M, size_t ef_construction);

  void release();

  /**
   * @brief 
   * 
   * @param sid 
   * @param vec 
   * @param persistence true means add element to disk only. If false, it will construct virtual graph and save to disk too.
   * @return int 
   */
  int add(node_t sid, const std::vector<double>& vec, bool persistence);
  int erase(size_t sid);
  int query(const std::vector<double>& vec, size_t topK, std::vector<uint64_t>& ids);
  int get(const std::vector<uint64_t>& ids, std::vector<std::vector<double> >& vecs);

private:
  int getLayer(double reverse_size);
  /**
   * @brief query near elements in layer
   * 
   * @param vec query element
   * @param ep entry point
   * @param topK the number of nearest elements
   * @param layer layer number
   * @return std::vector<uint64_t> 
   */
  std::vector<node_t> queryLayer(const std::vector<double>& vec, node_t ep, size_t topK, uint8_t layer);

  std::vector<node_t> knnSearch(const std::vector<double>& vec, size_t topK);

  std::vector<double> id2vector(node_t id);

  std::set<node_t> neighborhood(node_t id, uint8_t layer);

  std::set<node_t> selectNeighbors(node_t id, const std::map<node_t, std::vector<double>>& candidates, uint8_t count);

  bool addDisk(node_t id, uint8_t& level);

  struct InternalVertex {
    std::string _id;
    std::vector<float> _data;
  };

  struct LayerVertex {
    InternalVertex* _vertex;
    std::vector< InternalVertex* > _neighbors;
    LayerVertex* _nextLayerVertex;
  };

private:
  class NodeVisitor {
  public:
    void operator()(node_t, const node_info&) {}
  };

  class NodeLoader {
  public:
    bool load() const {
      return false;
    }
  };

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
  std::string _property;
  GStorageEngine* _storage;
  gql::GHNSWHeuristic _heuristic;
  gql::GHNSWAStarSelector* _selector;
  GVirtualNetwork* _network;
  
  std::default_random_engine _levelGenerator;
  
  /**
   * @brief the number of established connections
   */
  size_t _M;
  /**
   * @brief the size of the dynamic candidate list
   */
  size_t _ef;

  double _mult;
  /**
   * @brief normalization factor for level generation
   */
  double _revSize;
};