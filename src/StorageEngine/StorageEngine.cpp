#include "StorageEngine.h"
#include "Error.h"
#include "Type/Binary.h"
#include <regex>

using namespace mdbx;

GStorageEngine::GStorageEngine() {

}

GStorageEngine::~GStorageEngine() {
  if (_txn) {
    _txn.commit();
  }
  //_env.close();
}

int GStorageEngine::create(const char* filename) {
    env::geometry db_geometry;
    env_managed::create_parameters create_param;
    create_param.geometry=db_geometry;

    env::operate_parameters operator_param;
#define DEFAULT_MAX_MAPS  64
    operator_param.max_maps = DEFAULT_MAX_MAPS;

    _env = env_managed(std::string(filename), create_param, operator_param);
    return startTrans();
}

int GStorageEngine::openGraph(const char* name, GGraph*& pGraph) {
  if (!name) return ECODE_NULL_PTR;
  auto ptr = _mHandle.find(name);
  if (ptr == _mHandle.end()) {
      pGraph = new GGraph(_txn, name);
  }
  return ECode_Success;
}

int GStorageEngine::openGraph(const char* name)
{
  if (_mHandle.find(name) == _mHandle.end()) {
    GGraph* pGraph = nullptr;
    int ret = this->openGraph(name, pGraph);
    if (pGraph) {
      _mHandle[name] = pGraph;
    } else {
      return ECODE_NULL_PTR;
    }
  }
  _usedgraph = name;
  return ECode_Success;
}

void GStorageEngine::registGraphFeature(GGraph* pGraph, GVertexProptertyFeature* feature)
{
  pGraph->registPropertyFeature(feature);
}

GGraph* GStorageEngine::getGraph(const char* name)
{
  std::string gname;
  if (!name) gname = _usedgraph;
  else gname = name;
  std::vector<std::string> names = getGraphs();
  for (size_t i = 0; i < names.size(); i++)
  {
    if (names[i] == gname) {
      if (this->openGraph(gname.c_str()) != ECode_Success) {
        return nullptr;
      }
      return _mHandle[gname];
    }
  }
  return nullptr;
}

std::vector<std::string> GStorageEngine::getGraphs()
{
  mdbx::map_handle handle = open_schema(_txn);
  std::vector<std::string> v;
  mdbx::cursor_managed cursor = _txn.open_cursor(handle);
  mdbx::cursor::move_result result = cursor.to_first(false);
  while (result)
  {
    std::string name((char*)result.key.byte_ptr(), result.key.size());
    v.push_back(name);
    result = cursor.to_next(false);
  }
  return v;
}

int GStorageEngine::startTrans() {
  _txn = _env.start_write();
  if (!_txn) return ECODE_NULL_PTR;
  return ECode_Success;
}

int GStorageEngine::finishTrans() {
  _txn.commit();
  return ECode_Success;
}

int GStorageEngine::closeGraph(GGraph* pGraph) {
  if (pGraph) {
    delete pGraph;
  }
  return ECode_Success;
}

int GStorageEngine::dropGraph(GGraph* pGraph) {
  int res = pGraph->drop();
  if (!res) {
    delete pGraph;
    this->finishTrans();
    this->startTrans();
    return res;
  }
  return ECode_Fail;
}

int GStorageEngine::finishUpdate(GGraph* graph)
{
  return graph->finishUpdate(_txn);
}

int GStorageEngine::getNode(GGraph* graph, const VertexID& nodeid, std::function<int(const char*, void*, int, void*)> f)
{
  GVertex vertex = graph->getVertexById(nodeid);
  if (IS_INVALID_VERTEX(vertex._vertex)) return ECode_Success;
  for (auto itr = vertex._vertex.begin(), end = vertex._vertex.end(); itr != end; ++itr) {
    std::string k = itr.key();
    auto v = itr.value();
    //auto data = json_cast<v.type()>(v);
    switch (v.type()) {
    case nlohmann::json::value_t::string:
    {
      std::string data = v.get<std::string>();
      f(k.c_str(), (void*)(data.c_str()), (int)v.type(), nullptr);
      break;
    }
    case nlohmann::json::value_t::number_integer:
    {
      int data = v.get<int>();
      f(k.c_str(), (void*)&data, (int)v.type(), nullptr);
      break;
    }
    case nlohmann::json::value_t::number_unsigned:
    {
      unsigned int data = v.get<int>();
      f(k.c_str(), (void*)&data, (int)v.type(), nullptr);
      break;
    }
    case nlohmann::json::value_t::binary:
    {
      std::vector<uint8_t> bin = v.get<std::vector<uint8_t>>();
      std::string s = gql::base64_encode(bin);
      // f(k.c_str(), (void*)&(s[0]), (int)v.type(), nullptr);
      break;
    }
    default:
      break;
    }
  }
  return ECode_Success;
}

int GStorageEngine::getNode(GGraph* graph, const VertexID& nodeid, std::function<int(const char*, void*)> f)
{
  GVertex vertex = graph->getVertexById(nodeid);
  if (IS_INVALID_VERTEX(vertex._vertex)) return ECode_Success;
  if (vertex._hasBinary) {
    for (auto& item : vertex._vertex)
    {
      if (item.type() == nlohmann::json::value_t::binary) {
        nlohmann::json::binary_t bin = item;
        item = "b64'" + gql::base64_encode(bin) + "'";
      }
    }
  }
  std::string out = vertex._vertex.dump();
  out = std::regex_replace(out, std::regex("\"b64'([\\s\\S]+)'\"", std::regex_constants::ECMAScript), "b64'$1'");
  f(out.c_str(), nullptr);
  return ECode_Success;
}

std::vector<VertexID> GStorageEngine::getNodes(GGraph* graph)
{
  auto vertexes = graph->getVertex(_txn);
  std::vector<VertexID> ids;
  for (auto& vertex : vertexes) {
    VertexID vid(vertex.first);
    ids.emplace_back(vid);
  }
  return ids;
}

int GStorageEngine::dropNode(GGraph* graph, const VertexID& nodeid)
{
  graph->dropVertex(nodeid);
  return ECode_Success;
}

int GStorageEngine::makeDirection(GGraph* graph, const EdgeID& id, const VertexID& from, const VertexID& to, const char* name)
{
  // graph->updateEdge(id, name, "");
  // make relation and then save it
  graph->bind(id, from, to);
  return ECode_Success;
}
