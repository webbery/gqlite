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
#include "base/IDGenerator.h"
#include "StorageEngine.h"

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
   * save graph to disk
   */
  void save();
  /**
   * @brief 
   * 
   * @param sid 
   * @param vec 
   * @param persistence is true means add element to disk only. If false, it will construct virtual graph and save to disk too.
   * @return int 
   */
  int add(node_t sid, const std::vector<double>& vec);
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
   * @return 
   */
  std::list< std::tuple<node_t, double, std::vector<double>>> queryLayer(const std::vector<double>& vec, node_t ep, size_t topK, uint8_t layer);

  std::vector<node_t> knnSearch(const std::vector<double>& vec, size_t topK);

  std::vector<double> id2vector(node_t id);

  std::set<node_t> neighborhood(node_t id, uint8_t layer);

  std::set<node_t> selectNeighbors(const std::vector<double>& vec,
              const std::list<std::tuple<node_t, double, std::vector<double>>>& candidates, uint8_t count, int8_t level);

  /**
   * If id exist in disk, return false. Else add node to disk, put it in layer index, then return true.
   * @param level return the top layer that is added.
   */
  bool addDisk(node_t id, const std::vector<double>& value, int8_t level);
  void addNode2Edge(node_t nodeID, const std::vector<double>& vec, int8_t newLevel);

  uint8_t getTopLevel(node_t id);

  /**
   * Get top layer entry point
   */
  node_t entry(int8_t& level);

  /**
   * Construct virtual network from disk
   */
  void initNet();

  void readEachLayer(int8_t level, std::function<void(int8_t level, const mdbx::cursor::move_result& data)> f);

  void clipEdgeThenKeepNeighborSize(node_t node, int8_t level, size_t maxNeighbor);

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
  bool _updated;
  
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

  GIDGenerator<edge_t> _edgeIDGenerator;
  // each layer nodes, for quick search in graph
  std::array<std::set<node_t>, MAX_LAYER_SIZE> _cache;
};