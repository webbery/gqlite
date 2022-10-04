#include "operand/query/HNSW.h"
#include <random>
#include <float.h>
#include "base/system/Platform.h"
#include "gutil.h"
#include "gqlite.h"
#include "VirtualNetwork.h"
#include "Type/Vector.h"

namespace {
  using node_detail = std::tuple<node_t, double, std::vector<double>>;
  bool operator < (const node_detail& left, const node_detail& right) {
    return std::get<1>(left)<std::get<1>(right);
  }

  bool operator == (const node_detail& left, const node_detail& right) {
    return std::get<0>(left) == std::get<0>(right);
  }

  bool operator < (const node_detail& left, double dis) {
    return std::get<1>(left) < dis;
  }
}

GHNSW::GHNSW(GVirtualNetwork* network, GStorageEngine* store, const char* index_name, const char* prop, const char* graph /*= nullptr*/)
  :_network(network)
  ,_storage(store)
  , _property(prop)
  , _index(index_name)
  , _updated(false)
{
  if (!_storage->isOpen()) {
    StoreOption option;
    option.compress = 1;
    _storage->open(graph, option);
  }
  for (uint8_t level = 0; level < MAX_LAYER_SIZE; ++level) {
    std::string name = _index + ":" + std::to_string(level);
    _storage->addMap(name, KeyType::Integer);
  }
  _storage->addMap(prop, KeyType::Integer);
  _selector = new gql::GHNSWAStarSelector(_heuristic);

  unsigned int seed = 120;
  _levelGenerator.seed(seed);

  init(16, 16);
  int8_t level = MAX_LAYER_SIZE - 1;
  readEachLayer(level, [&cache = this->_cache](int8_t level, const mdbx::cursor::move_result& data) {
    node_t k = *(node_t*)data.key.byte_ptr();
    for (int8_t lvl = level; lvl >= 0; --lvl) {
      cache[lvl].insert(k);
    }
    node_t* v = (node_t*)data.value.byte_ptr();
    int cnt = data.value.size() / sizeof(node_t);
    //printf("layer %d id %d has %d count\n", level, k, cnt);
  });
}

void GHNSW::init(size_t M, size_t ef_construction)
{
  _M = M;
  _ef = ef_construction;
  _revSize = log2(1.0 * M);
  _mult = 1 / _revSize;
}

GHNSW::~GHNSW()
{
  delete _selector;
  //for (auto item : _mHNSWs) {
    //if (item.second->_instance) delete item.second->_instance;
    //if (item.second->_pSpace) delete item.second->_pSpace;
  //}
}

int GHNSW::getLayer(double reverse_size)
{
  std::uniform_real_distribution<double> distribution(0.0, 1.0);
  double v = distribution(_levelGenerator);
  double r = v * reverse_size;
  return (int)r;
}

std::list<std::tuple<node_t, double, std::vector<double>>> GHNSW::queryLayer(const std::vector<double>& q, node_t ep, size_t topK, uint8_t layer)
{
  std::set<node_t> visited;
  auto epVec = id2vector(ep);
  double dist = gql::distance2(q, epVec);
  std::map<double, node_t> candidates;
  candidates[dist] = ep;
  std::map<double, node_t> W;
  W[dist] = ep;

  std::list<node_detail> elements;
  while (candidates.size() && W.size()) {
    node_t elm = candidates.begin()->second;
    candidates.erase(candidates.begin());
    std::vector<double> v = id2vector(elm);
    node_t farest = W.rbegin()->second;
    std::vector<double> f = id2vector(farest);
    double farest_distance = gql::distance2(q, f);
    if (gql::distance2(v, q) > farest_distance) break;

    auto neighbors = neighborhood(elm, layer);
    neighbors.insert(elm);
    for (auto nitr = neighbors.begin(); nitr != neighbors.end(); ++nitr) {
      node_t neighbor = *nitr;
      if (visited.count(neighbor)) continue;
      visited.insert(neighbor);

      farest = W.rbegin()->second;
      auto e = id2vector(neighbor);
      f = id2vector(farest);
      double near_distance = gql::distance2(e, q);
      if (near_distance < farest_distance + DBL_EPSILON || W.size() < topK) {
        candidates[near_distance] = neighbor;
        W[near_distance] = neighbor;
        node_detail detail = std::make_tuple(neighbor, near_distance, e);
        addUniqueDataAndSort<node_detail, std::list<node_detail>>(elements, detail);
        if (W.size() > topK) {
          auto itr = std::prev(W.end());
          node_detail item = std::make_tuple(itr->second, itr->first, std::vector<double>());
          eraseSortedData(elements, item);
          W.erase(itr);
        }
      }
    }
  }
  return elements;
}

std::vector<node_t> GHNSW::knnSearch(const std::vector<double>& vec, size_t topK)
{
  int8_t level = MAX_LAYER_SIZE - 1;
  auto indxCursor = _storage->getMapCursor(_index + ":" + std::to_string(level));
  auto data = indxCursor.to_first(false);
  std::vector<node_t> nodes;
  while (!data) {
    if (level == 0) return nodes;
    indxCursor = _storage->getMapCursor(_index + ":" + std::to_string(--level));
    data = indxCursor.to_first(false);
  }
  
  std::vector<node_t> revIndexes;
  node_t* start = (node_t*)data.value.byte_ptr();
  //printf("size: %d, unit: %d, id: %d\n", data.value.size(), sizeof(node_t), *start);
  revIndexes.insert(revIndexes.end(), start, start + data.value.size() / sizeof(node_t));
  if (revIndexes.size() == 0) return nodes;
  node_t nearest = revIndexes[0];
  while (level > 0) {
    std::list<node_detail> elements = queryLayer(vec, nearest, topK, level);
    if (elements.size()) {
      auto elem = elements.front();
      nearest = std::get<0>(elem);
    }
    --level;
  }
  auto details = queryLayer(vec, nearest, topK, level);
  for (auto itr = details.begin(); itr != details.end(); ++itr) {
    nodes.emplace_back(std::get<0>(*itr));
  }
  return nodes;
}

std::vector<double> GHNSW::id2vector(node_t id)
{
  std::string pt;
  std::vector<double> epVector;
  if (_storage->read(_property, static_cast<uint64_t>(id), pt) == ECode_Success) {
    epVector.insert(epVector.end(), (double*)pt.data(), (double*)pt.data() + pt.size() / sizeof(double));
  }
  return epVector;
}

std::set<node_t> GHNSW::neighborhood(node_t id, uint8_t layer)
{
  std::set<node_t> n;
  if (_network->node_size()) {
    _network->neighbors(id, n, layer);
  }
  else {
    // try to search from disk
    std::string value;
    if (_storage->read(_index + ":" + std::to_string(layer), static_cast<uint64_t>(id), value) != ECode_DATUM_Not_Exist) {
      node_t* nodes = (node_t*)value.data();
      n.insert(nodes, nodes + value.size() / sizeof(node_t));
    }
    //printf("neighbor: %ld\n", n.size());
  }
  return n;
}

std::set<node_t> GHNSW::selectNeighbors(const std::vector<double>& vec, const std::list<node_detail>& candidates, uint8_t count, int8_t level)
{
  node_t ep = entry(level);
  auto nodes = queryLayer(vec, ep, count, level);
  std::set<node_t> n;
  for (auto itr = nodes.begin(); itr != nodes.end(); ++itr) {
    n.insert(std::get<0>(*itr));
  }
  return n;
}

bool GHNSW::addDisk(node_t sid, const std::vector<double>& vec, int8_t level)
{
  std::string value;
  if (_storage->read(_property, (uint64_t)sid, value) == ECode_DATUM_Not_Exist) {
    for (int8_t lvl = 0; lvl <= level; ++lvl) {
      _cache[lvl].insert(sid);
    }
    _storage->write(_property, (uint64_t)sid, (void*)vec.data(), vec.size() * sizeof(double));
    return true;
  }
  return false;
}

void GHNSW::addNode2Edge(node_t nodeID, const std::vector<double>& vec, int8_t newLevel)
{
  // get entry point from top layer
  int8_t startLevel;
  node_t ep = entry(startLevel);
  assert(startLevel >= 0);
  for (uint8_t level = startLevel; level < newLevel + 1; ++level)
  {
    auto W = queryLayer(vec, ep, 1, level);
    if (W.size() == 0) {
      startLevel = level;
      break;
    }
    node_detail node = W.front();
    ep = std::get<0>(node);
  }
  for (int8_t level = std::min(startLevel, newLevel); level >= 0; --level) {
    std::list<std::tuple<node_t, double, std::vector<double>>> W;
    //TEST_TIME(
      W = queryLayer(vec, ep, _ef, (uint8_t)level);
    //);
    std::map<node_t, double> mNodes;
    for (auto itr = W.begin(); itr != W.end(); ++itr) {
      mNodes[std::get<0>(*itr)] = std::get<1>(*itr);
    }
    std::set<node_t> neighbors = selectNeighbors(vec, W, _M, level);
    for (auto nid : neighbors) {
      // hnsw has no circle loop
      if (nodeID == nid) continue;
      edge_t eid = _edgeIDGenerator.generate();
      clipEdgeThenKeepNeighborSize(nodeID, level, _M - 1);
      clipEdgeThenKeepNeighborSize(nid, level, _M - 1);
      _network->addEdge(eid, nodeID, nid, {}, mNodes[nid], level);
      _network->addEdge(eid, nid, nodeID, {}, mNodes[nid], level);
    }
  }
}

uint8_t GHNSW::getTopLevel(node_t id)
{
  for (uint8_t level = 0; level < MAX_LAYER_SIZE; ++level)
  {
    std::string value;
    if (_storage->read(_index + ":" + std::to_string(level + 1), (uint64_t)id, value) == ECode_DATUM_Not_Exist) {
      return level + 1;
    }
  }
  return 0;
}

node_t GHNSW::entry(int8_t& level)
{
  level = MAX_LAYER_SIZE - 1;
  do 
  {
    auto cursor = _cache[level].begin();
    if (cursor != _cache[level].end()) {
      return *cursor;
    }
  } while (--level >= 0);
  return (node_t)virtual_graph_t::no_node;
}

void GHNSW::initNet()
{
  int8_t level = MAX_LAYER_SIZE - 1;
  // init nodes
  readEachLayer(level, [network = this->_network](int8_t level, const mdbx::cursor::move_result& data) {
    node_t k = *(node_t*)data.key.byte_ptr();
    network->addNode(k, {}, nlohmann::json(), level);
    });
  // init edges
  edge_t eid = _edgeIDGenerator.generate();
  std::set<std::pair<int8_t, node_t>> visited;
  readEachLayer(level, 
    [&visited , &eid, network = this->_network, &edgeIDGenerator = this->_edgeIDGenerator](int8_t level, const mdbx::cursor::move_result& data) {
    node_t k = *(node_t*)data.key.byte_ptr();
    node_t* n = (node_t*)data.value.byte_ptr();
    size_t cnt = data.value.size() / sizeof(node_t);
    for (size_t indx = 0; indx < cnt; ++indx) {
      if (visited.count({ level, n[indx] })) continue;
      network->addEdge(eid, n[indx], k, {}, {}, level);
      if (network->addEdge(eid, k, n[indx], {}, {}, level) == ECode_Success) {
        eid = edgeIDGenerator.generate();
      }
    }
    visited.insert({ level, k });
  });
}

void GHNSW::readEachLayer(int8_t level, std::function<void(int8_t level, const mdbx::cursor::move_result& data)> f)
{
  auto cursor = _storage->getMapCursor(_index + ":" + std::to_string(level));
  auto data = cursor.to_first(false);
  int8_t curLevel = level;
  auto next = [&]() {
    while (!data) {
      if (curLevel == 0) return false;
      --curLevel;
      cursor = _storage->getMapCursor(_index + ":" + std::to_string(curLevel));
      data = cursor.to_first(false);
    }
    return true;
  };
  do {
    if (!next()) break;
    f(curLevel, data);
    data = cursor.to_next(false);
  } while (true);
}

void GHNSW::clipEdgeThenKeepNeighborSize(node_t node, int8_t level, size_t maxNeighbor)
{
  std::set<edge_t> neighbors;
  _network->neighbors(node, neighbors, level);
  if (neighbors.size() >= maxNeighbor) {
    double f = 0;
    edge_t eid = (edge_t)GMap<uint64_t, uint64_t>::no_edge;
    std::for_each(neighbors.begin(), neighbors.end(), [&](edge_t id) {
      double d = _network->edge_info(id);
      if (d >= f) {
        f = d;
        eid = id;
      }
    });
    if (eid != (edge_t)GMap<uint64_t, uint64_t>::no_edge) {
      _network->deleteEdge(eid, false);
    }
    else {
      printf("WARN: node %ld not delete edge\n", node);
    }
  }
}

void GHNSW::release()
{
  save();
}

void GHNSW::save()
{
  if (_updated) {
    for (int8_t level = 0; level < MAX_LAYER_SIZE; ++level)
    {
      auto& nodes = _cache[level];
      for (node_t id : nodes) {
        std::set<node_t> neighbors;
        if (_network->neighbors(id, neighbors, level)) {
          std::vector<node_t> v(neighbors.begin(), neighbors.end());
          //printf("save node %ld, level: %d, neighbor: %ld\n", id, level, neighbors.size());
          _storage->write(_index + ":" + std::to_string(level), static_cast<uint64_t>(id), &(v[0]), v.size() * sizeof(node_t));
        }
      }
    }
  }
}

int GHNSW::add(node_t sid, const std::vector<double>& vec)
{
  if (_network->node_size() == 0) {
    initNet();
  }
  // check sid exist in disk or not
  int8_t level = getLayer(_revSize);
  if (addDisk(sid, vec, level)) {
    _network->addNode(sid, {}, {}, level);
    addNode2Edge(sid, vec, level);
    _updated = true;
  };
  return 0;
}

int GHNSW::erase(size_t sid)
{
  if (_network->node_size() == 0) {
    initNet();
  }
  _updated = true;
  return 0;
}

int GHNSW::query(const std::vector<double>& vec, size_t topK, std::vector<uint64_t>& ids)
{
  auto nodes = knnSearch(vec, topK);
  std::for_each(nodes.begin(), nodes.end(), [&ids](node_t node_id) {
    ids.emplace_back((uint64_t)node_id);
    });
  return 0;
}

int GHNSW::get(const std::vector<uint64_t>& ids, std::vector<std::vector<double> >& vecs)
{
  for (auto& id : ids) {
    auto vec = id2vector((node_t)id);
    vecs.emplace_back(vec);
  }
  return 0;
}

