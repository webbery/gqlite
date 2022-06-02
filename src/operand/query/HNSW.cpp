#include "operand/query/HNSW.h"
#include <random>

GHNSW::GHNSW(NSWDistance dis)
  :_distype(dis)
  , _pSpace(nullptr)
  , _instance(nullptr)
{
  
}

GHNSW::~GHNSW()
{
  if (_instance) delete _instance;
  if (_pSpace) delete _pSpace;
}

GHNSW* GHNSW::load(NSWDistance dis)
{
  static GHNSW* _instance = new GHNSW(dis);
  return _instance;
}

void GHNSW::release(GHNSW* hnsw)
{
  if (hnsw) delete hnsw;
}

int GHNSW::caculateLayer()
{
  static std::default_random_engine re;
  static std::uniform_real_distribution<float> distribution(0, 1);
  float value = distribution(re);
  // 4 3 2 1
  if (value < 0.4) return 0;
  if (value >= 0.4 && value < 0.7) return 1;
  if (value >= 0.7 && value < 0.9) return 2;
  return 3;
}

int GHNSW::add(size_t sid, const std::vector<float>& vec)
{
  if (_pSpace == nullptr) {
    _pSpace = new hnswlib::L2Space(vec.size());
    _instance = new hnswlib::HierarchicalNSW(_pSpace, MAX_INDEX_COUNT);
  }
  _instance->addPoint((void*)vec.data(), sid);
  return 0;
}

int GHNSW::erase(size_t sid)
{
  _instance->markDelete(sid);
  return 0;
}

int GHNSW::query(const std::vector<float>& vec, size_t topK, std::vector<size_t>& ids)
{
  std::priority_queue<std::pair<float, size_t>> result = _instance->searchKnn((void*)vec.data(), topK);
  while (result.size()) {
    std::pair<float, size_t> top = result.top();
    ids.push_back(top.second);
    result.pop();
  }
  return 0;
}

int GHNSW::get(const std::vector<size_t>& ids, std::vector<std::vector<float> >& vecs)
{
  for (auto& id : ids) {
    std::vector<float> data = _instance->getDataByLabel<float>(id);
    vecs.emplace_back(data);
  }
  return 0;
}
