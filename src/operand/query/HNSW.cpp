#include "operand/query/HNSW.h"
#include <random>
#include "base/system/Platform.h"
#include "gutil.h"
#include "gqlite.h"
#include "VirtualNetwork.h"
#include "StorageEngine.h"
#include "Type/Vector.h"

GHNSW::GHNSW(GVirtualNetwork* network, GStorageEngine* store, const char* index_name, const char* prop, const char* graph /*= nullptr*/)
  :_network(network)
  ,_storage(store)
  , _property(prop)
  , _index(index_name)
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

  _selector = new gql::GHNSWAStarSelector(_heuristic);

  unsigned int seed = 120;
  _levelGenerator.seed(seed);

}

void GHNSW::init(size_t M, size_t ef_construction)
{
  _M = M;
  _ef = ef_construction;
  _revSize = log(1.0 * M);
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
  double r = -log(distribution(_levelGenerator)) * reverse_size;
  return (int)r;
}

std::vector<node_t> GHNSW::queryLayer(const std::vector<double>& q, node_t ep, size_t topK, uint8_t layer)
{
  std::list<node_t> candidates;
  candidates.push_back(ep);
  std::map<double, node_t> nearest_neighbors;
  auto neighbors = neighborhood(ep, layer);
  for (node_t neighbor : neighbors) {
    std::vector<double> nv = id2vector(neighbor);
    double dist = gql::distance2(q, nv);
    nearest_neighbors[dist] = ep;
  }

  while (candidates.size()) {
    node_t elm = candidates.front();
    candidates.pop_front();
    std::vector<double> v = id2vector(elm);
    node_t farest = nearest_neighbors.rbegin()->second;
    std::vector<double> f = id2vector(farest);
    double farest_distance = gql::distance2(q, f);
    if (gql::distance2(v, q) > farest_distance) break;

    auto neighbors = neighborhood(elm, layer);
    for (node_t neighbor : neighbors) {
      if (_network->isVisited(neighbor)) continue;
      //_network->visit()
      farest = nearest_neighbors.rbegin()->second;
      auto e = id2vector(neighbor);
      f = id2vector(farest);
      double near_distance = gql::distance2(e, q);
      if (near_distance < farest_distance || nearest_neighbors.size() < topK) {
        candidates.push_back(neighbor);
        nearest_neighbors[near_distance] = neighbor;
        if (nearest_neighbors.size() > topK) {
          nearest_neighbors.erase(std::prev(nearest_neighbors.end()));
        }
      }
    }
  }
  std::vector<node_t> elements(nearest_neighbors.size());
  size_t idx = 0;
  for (auto& pair : nearest_neighbors) {
    elements[idx++] = pair.second;
  }
  return elements;
}

std::vector<node_t> GHNSW::knnSearch(const std::vector<double>& vec, size_t topK)
{
  int8_t level = MAX_LAYER_SIZE - 1;
  auto indxCursor = _storage->getCursor(_index + ":" + std::to_string(level));
  auto data = indxCursor.to_first(false);
  std::vector<node_t> revIndexes;
  node_t* start = (node_t*)data.value.byte_ptr();
  revIndexes.insert(revIndexes.end(), start, start + data.value.size() / sizeof(node_t));
  node_t nearest = revIndexes[0];
  while (level > 0) {
    std::vector<node_t> elements = queryLayer(vec, nearest, topK, level);
    nearest = elements[0];
    --level;
  }
  return queryLayer(vec, nearest, topK, 0);
}

std::vector<double> GHNSW::id2vector(node_t id)
{
  std::string pt;
  _storage->read(_property, static_cast<uint64_t>(id), pt);
  std::vector<double> epVector;
  epVector.insert(epVector.end(), pt.data(), pt.data() + pt.size());
  return epVector;
}

std::set<node_t> GHNSW::neighborhood(node_t id, uint8_t layer)
{
  return _network->neighbors(id);
}

std::set<node_t> GHNSW::selectNeighbors(node_t id, const std::map<node_t, std::vector<double>>& candidates, uint8_t count)
{
  std::set<node_t> n;
  return n;
}

bool GHNSW::addDisk(node_t sid, uint8_t& level)
{
  std::string value;
  if (_storage->read(_index + ":0", (uint64_t)sid, value) == ECode_DATUM_Not_Exist) {
    level = getLayer(_revSize);
    for (uint8_t idx = 0; idx <= level; ++idx) {
      _storage->write(_index + ":" + std::to_string(idx), static_cast<uint64_t>(sid), value.data(), value.size());
    }
    return true;
  }
  for (; level < MAX_LAYER_SIZE; ++level)
  {
    if (_storage->read(_index + ":" + std::to_string(level + 1), (uint64_t)sid, value) == ECode_DATUM_Not_Exist) {
      break;
    }
  }
  return false;
}

void GHNSW::release()
{
}

int GHNSW::add(node_t sid, const std::vector<double>& vec, bool persistence)
{
  // check sid exist in disk or not
  uint8_t L = 0;
  uint8_t newLevel = 0;
  if (addDisk(sid, L)) newLevel = L;
  else newLevel = getLayer(_revSize);
  for (uint8_t level =L; level < newLevel + 1; ++level)
  {
  }
  return 0;
}

int GHNSW::erase(size_t sid)
{
  //_activeHNSW->_instance->markDelete(sid);
  return 0;
}

int GHNSW::query(const std::vector<double>& vec, size_t topK, std::vector<uint64_t>& ids)
{
  //std::priority_queue<std::pair<float, size_t>> result = _activeHNSW->_instance->searchKnn((void*)vec.data(), topK);
  //while (result.size()) {
  //  std::pair<float, size_t> top = result.top();
  //  ids.push_back(top.second);
  //  result.pop();
  //}
  return 0;
}

int GHNSW::get(const std::vector<uint64_t>& ids, std::vector<std::vector<double> >& vecs)
{
  for (auto& id : ids) {
    //std::vector<float> data = _activeHNSW->_instance->getDataByLabel<float>(id);
    //vecs.emplace_back(data);
  }
  return 0;
}

