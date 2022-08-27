#include "operand/query/HNSW.h"
#include <random>
#include "base/system/Platform.h"
#include "gutil.h"
#include "gqlite.h"
#include "StorageEngine.h"

GHNSW::GHNSW(GStorageEngine* storage, const char* index_name, const char* graph /*= nullptr*/)
  :_storage(storage), _index(index_name)
{
  if (!_storage->isOpen()) {
    StoreOption option;
    option.compress = 1;
    _storage->open(graph, option);
  }
  std::string level_0(_index);
  std::string level_1 = level_0 + ":1";
  std::string level_2 = level_0 + ":2";
  std::string level_3 = level_0 + ":3";
  _storage->addMap(level_3, KeyType::Integer);
  _storage->addMap(level_2, KeyType::Integer);
  _storage->addMap(level_1, KeyType::Integer);
  _storage->addMap(level_0, KeyType::Integer);
}

GHNSW::~GHNSW()
{
  //for (auto item : _mHNSWs) {
    //if (item.second->_instance) delete item.second->_instance;
    //if (item.second->_pSpace) delete item.second->_pSpace;
  //}
}

void GHNSW::load()
{
  //if (_mHNSWs.count(index_file) == 0 ) {
  //  std::vector<std::string> tokens = gql::split(index_file.c_str(), "_");
  //  _activeGraph = tokens[0];
  //  if (isFileExist(index_file.c_str())) {
  //    initHNSW(index_file, atoi(tokens[1].c_str()));
  //    return ECode_Success;
  //  }
  //}
}

void GHNSW::release(GHNSW* hnsw)
{
  if (hnsw) delete hnsw;
}

int GHNSW::add(uint64_t sid, const std::vector<double>& vec)
{
  // check sid exist in disk or not
  std::string value;

  if (_storage->read(_index, sid, value) == ECode_DATUM_Not_Exist) {

  }
  else {
    // update vector
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

bool GHNSW::initHNSW(const std::string& key, size_t dim)
{
  _index = key;
  //HNSW* pHNSW = new HNSW();
  //pHNSW->_pSpace = new hnswlib::L2Space(dim);
  //pHNSW->_instance = new hnswlib::HierarchicalNSW<float>(pHNSW->_pSpace, MAX_INDEX_COUNT);
  //_mHNSWs[_activeFilename] = pHNSW;
  //_activeHNSW = pHNSW;
  return true;
}
