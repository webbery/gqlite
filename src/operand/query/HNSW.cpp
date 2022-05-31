#include "operand/query/HNSW.h"
#include <random>

GHNSW::GHNSW(NSWDistance dis)
  :_distype(dis)
{
  hnswlib::L2Space l2(MAX_DIMENSION);
  _instance = new hnswlib::HierarchicalNSW(&l2, MAX_INDEX_COUNT);
}

GHNSW::~GHNSW()
{
  if (_instance) delete _instance;
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

int GHNSW::add(const std::string& sid, const std::vector<float>& vec)
{
  
  return 0;
}

int GHNSW::erase(const std::string& sid)
{
  return 0;
}

int GHNSW::query(const std::vector<float>& vec, size_t topK, std::vector<std::string>& ids)
{
  return 0;
}

int GHNSW::get(const std::vector<std::string>& ids, std::vector<std::vector<float> >& vecs)
{
  for (auto& id : ids) {

  }
  return 0;
}
