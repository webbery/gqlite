#include "operand/query/HNSW.h"
#include <random>
#include "base/system/Platform.h"
#include "gutil.h"
#include "gqlite.h"

GHNSWManager::GHNSWManager(const char* graph)
  :_activeGraph(graph)
  //,_activeHNSW(nullptr)
{
  
}

GHNSWManager::~GHNSWManager()
{
  //for (auto item : _mHNSWs) {
    //if (item.second->_instance) delete item.second->_instance;
    //if (item.second->_pSpace) delete item.second->_pSpace;
  //}
}

int GHNSWManager::load(const std::string& index_file)
{
  //if (_mHNSWs.count(index_file) == 0 ) {
  //  std::vector<std::string> tokens = gql::split(index_file.c_str(), "_");
  //  _activeGraph = tokens[0];
  //  if (isFileExist(index_file.c_str())) {
  //    initHNSW(index_file, atoi(tokens[1].c_str()));
  //    return ECode_Success;
  //  }
  //}
  return ECode_Index_File_Not_Exist;
}

int GHNSWManager::save(const std::string& index_file)
{
  //if (_activeHNSW) {
    //_activeHNSW->_instance->saveIndex(index_file);
  //}
  return 0;
}

void GHNSWManager::release(GHNSWManager* hnsw)
{
  if (hnsw) delete hnsw;
}

int GHNSWManager::add(size_t sid, const std::vector<float>& vec)
{
  //if (_activeFilename.size() == 0 || _mHNSWs.count(_activeFilename) == 0) {
    std::string filename = _activeGraph + "_" + std::to_string(vec.size()) + "_0.indx";
    initHNSW(filename, vec.size());
  //}
  //_activeHNSW->_instance->addPoint((void*)vec.data(), sid);
  return 0;
}

int GHNSWManager::erase(size_t sid)
{
  //_activeHNSW->_instance->markDelete(sid);
  return 0;
}

int GHNSWManager::query(const std::vector<float>& vec, size_t topK, std::vector<size_t>& ids)
{
  //std::priority_queue<std::pair<float, size_t>> result = _activeHNSW->_instance->searchKnn((void*)vec.data(), topK);
  //while (result.size()) {
  //  std::pair<float, size_t> top = result.top();
  //  ids.push_back(top.second);
  //  result.pop();
  //}
  return 0;
}

int GHNSWManager::get(const std::vector<size_t>& ids, std::vector<std::vector<float> >& vecs)
{
  for (auto& id : ids) {
    //std::vector<float> data = _activeHNSW->_instance->getDataByLabel<float>(id);
    //vecs.emplace_back(data);
  }
  return 0;
}

bool GHNSWManager::initHNSW(const std::string& key, size_t dim)
{
  _activeFilename = key;
  //HNSW* pHNSW = new HNSW();
  //pHNSW->_pSpace = new hnswlib::L2Space(dim);
  //pHNSW->_instance = new hnswlib::HierarchicalNSW<float>(pHNSW->_pSpace, MAX_INDEX_COUNT);
  //_mHNSWs[_activeFilename] = pHNSW;
  //_activeHNSW = pHNSW;
  return true;
}
