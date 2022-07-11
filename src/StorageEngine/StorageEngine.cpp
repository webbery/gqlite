#include "StorageEngine.h"
#include <regex>
#define GRAPH_EXCEPTION_CATCH(expr) try{\
  expr;\
}catch(std::exception& err) {printf("exception %s [%d]: %s\n", __FILE__, __LINE__, err.what());}
#define DB_SCHEMA   "gql_schema"
#define SCHEMA_BASIC  "basic"

using namespace mdbx;
namespace {
  mdbx::slice get(mdbx::txn_managed& txn, mdbx::map_handle& map, const std::string& key)
  {
    mdbx::slice k(key.data(), key.size());
    mdbx::slice absent;
    return txn.get(map, k, absent);
  }

  mdbx::slice get(mdbx::txn_managed& txn, mdbx::map_handle& map, int32_t key)
  {
    mdbx::slice k(&key, sizeof(int32_t));
    mdbx::slice absent;
    return txn.get(map, k, absent);
  }

  int put(mdbx::txn_managed& txn, mdbx::map_handle& map, const std::string& key, mdbx::slice value)
  {
    mdbx::slice k(key.data(), key.size());
    txn.put(map, k, value, mdbx::upsert);
    return 0;
  }

  int put(mdbx::txn_managed& txn, mdbx::map_handle& map, int32_t key, mdbx::slice value)
  {
    mdbx::slice k(&key, sizeof(int32_t));
    txn.put(map, k, value, mdbx::upsert);
    return 0;
  }
}

GStorageEngine::GStorageEngine() {

}

GStorageEngine::~GStorageEngine() {
  close();
  if (_env) _env.close();
}

int GStorageEngine::open(const char* filename) {
  if (_env) {
    this->close();
    _env.close();
    //mdbx::env::info info = _env.get_info();
    //info.
  }
  env::geometry db_geometry;
  env_managed::create_parameters create_param;
  create_param.geometry=db_geometry;

  env::operate_parameters operator_param;
#define DEFAULT_MAX_PROPS  64
  operator_param.max_maps = DEFAULT_MAX_PROPS;

  _env = env_managed(std::string(filename), create_param, operator_param);
  int ret = startTrans();
  mdbx::map_handle handle = openSchema();
  mdbx::slice data = ::get(_txn, handle, SCHEMA_BASIC);
  if (data.size()) {
    std::vector<uint8_t> v(data.byte_ptr(), data.byte_ptr() + data.size());
    _schema = nlohmann::json::from_cbor(v);
  }
  // _env.close_map(handle);
  return ret;
}

void GStorageEngine::close() {
  if (_txn) {
    if (!_schema.empty()) {
      mdbx::map_handle handle = openSchema();
      std::vector<uint8_t> v = nlohmann::json::to_cbor(_schema);
      mdbx::slice data(v.data(), v.size());
      ::put(_txn, handle, SCHEMA_BASIC, data);
      // _env.close_map(handle);
    }
    // for (auto itr = _mHandle.begin(); itr != _mHandle.end(); ++itr) {
    //   _env.close_map(itr->second);
    // }
    // _mHandle.clear();
    _txn.commit();
  }
}

mdbx::map_handle GStorageEngine::openSchema() {
  mdbx::map_handle schema;
  GRAPH_EXCEPTION_CATCH(schema = _txn.create_map(DB_SCHEMA, mdbx::key_mode::usual, mdbx::value_mode::single));
  return schema;
}

void GStorageEngine::addProp(const std::string& prop, PropertyInfo type) {
  if (!isPropExist(prop)) {
    nlohmann::json info;
    info["name"] = prop;
    uint8_t value = *(uint8_t*)&type;
    info["info"] = value;
    _schema["prop"].push_back(info);
  }
}

bool GStorageEngine::isPropExist(const std::string& prop) {
  if (_schema.empty()) return false;
  const auto& props = _schema["prop"];
  for (auto itr = props.begin(); itr != props.end(); ++itr) {
    if ((*itr)["name"] == prop) return true;
  }
  return false;
}

nlohmann::json GStorageEngine::getProp(const std::string& prop)
{
  const auto& props = _schema["prop"];
  for (auto itr = props.begin(); itr != props.end(); ++itr) {
    if ((*itr)["name"] == prop) return *itr;
  }
  assert(false);
}

mdbx::map_handle GStorageEngine::getOrCreateHandle(const std::string& prop, mdbx::key_mode mode) {
  if (_mHandle.count(prop) == 0) {
    mdbx::map_handle propMap;
    GRAPH_EXCEPTION_CATCH(propMap = _txn.create_map(prop, mode, mdbx::value_mode::single));
    _mHandle[prop] = propMap;
  }
  return _mHandle[prop];
}

int GStorageEngine::write(const std::string& prop, const std::string& key, void* value, size_t len) {
  assert(isPropExist(prop));
  mdbx::slice data(value, len);
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::usual);
  ::put(_txn, handle, key, data);
  return ECode_Success;
}

int GStorageEngine::read(const std::string& prop, const std::string& key, std::string& value) {
  assert(isPropExist(prop));
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::usual);
  mdbx::slice data = ::get(_txn, handle, key);
  value.assign((char*)data.data(), data.size());
  return ECode_Success;
}

int GStorageEngine::write(const std::string& prop, uint64_t key, void* value, size_t len) {
  assert(isPropExist(prop));
  mdbx::slice data(value, len);
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::ordinal);
  ::put(_txn, handle, key, data);
  return ECode_Success;

}
int GStorageEngine::read(const std::string& prop, uint64_t key, std::string& value) {
  assert(isPropExist(prop));
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::ordinal);
  mdbx::slice data = ::get(_txn, handle, key);
  value.assign((char*)data.data(), data.size());
  return ECode_Success;
}

void GStorageEngine::addAttribute(const std::string& key, const std::string& value) {
  _schema[key] = value;
}

GStorageEngine::cursor GStorageEngine::getCursor(const std::string& prop)
{
  assert(isPropExist(prop));
  mdbx::key_mode mode = mdbx::key_mode::ordinal;
  auto pp = getProp(prop);
  auto c = (uint8_t)pp["info"];
  PropertyInfo info = { 0 };
  size_t n = sizeof(PropertyInfo);
  std::memcpy(&info, &c, sizeof(PropertyInfo));
  mdbx::map_handle handle;
  if (info.key_type == 0) {
    handle = getOrCreateHandle(prop, mdbx::key_mode::ordinal);
  }
  else {
    handle = getOrCreateHandle(prop, mdbx::key_mode::usual);
  }
  return _txn.open_cursor(handle);
}

void GStorageEngine::registGraphFeature(GGraphInstance* pGraph, GVertexProptertyFeature* feature)
{
  //pGraph->registPropertyFeature(feature);
}

// GGraphInstance* GStorageEngine::getGraph(const char* name)
// {
//   std::string gname;
//   if (!name) gname = _usedgraph;
//   else gname = name;
//   std::vector<std::string> names = getGraphs();
//   for (size_t i = 0; i < names.size(); i++)
//   {
//     if (names[i] == gname) {
//       if (this->openGraph(gname.c_str()) != ECode_Success) {
//         return nullptr;
//       }
//       return _mHandle[gname];
//     }
//   }
//   return nullptr;
// }

// std::vector<std::string> GStorageEngine::getGraphs()
// {
//   mdbx::map_handle handle = open_schema(_txn);
//   if (!handle) printf("222222222\n");
//   std::vector<std::string> v;
//   mdbx::cursor_managed cursor = _txn.open_cursor(handle);
//   mdbx::cursor::move_result result = cursor.to_first(false);
//   while (result)
//   {
//     std::string name((char*)result.key.byte_ptr(), result.key.size());
//     v.push_back(name);
//     result = cursor.to_next(false);
//   }
//   return v;
// }

int GStorageEngine::startTrans() {
  _txn = _env.start_write();
  if (!_txn) return ECODE_NULL_PTR;
  return ECode_Success;
}

int GStorageEngine::finishTrans() {
  _txn.commit();
  return ECode_Success;
}

// int GStorageEngine::closeGraph(GGraphInstance* pGraph) {
//   if (pGraph) {
//     delete pGraph;
//   }
//   return ECode_Success;
// }

// int GStorageEngine::dropGraph(GGraphInstance* pGraph) {
//   int res = pGraph->drop();
//   if (!res) {
//     delete pGraph;
//     this->finishTrans();
//     this->startTrans();
//     return res;
//   }
//   return ECode_Fail;
// }

// int GStorageEngine::finishUpdate(GGraphInstance* graph)
// {
//   return graph->finishUpdate(_txn);
// }

// int GStorageEngine::getNode(GGraphInstance* graph, const VertexID& nodeid, std::function<int(const char*, void*, int, void*)> f)
// {
//   GVertex vertex = graph->getVertexById(nodeid);
//   if (IS_INVALID_VERTEX(vertex.property())) return ECode_Success;
//   for (auto itr = vertex.property().begin(), end = vertex.property().end(); itr != end; ++itr) {
//     std::string k = itr.key();
//     auto v = itr.value();
//     //auto data = json_cast<v.type()>(v);
//     switch (v.type()) {
//     case nlohmann::json::value_t::string:
//     {
//       std::string data = v.get<std::string>();
//       f(k.c_str(), (void*)(data.c_str()), (int)v.type(), nullptr);
//       break;
//     }
//     case nlohmann::json::value_t::number_integer:
//     {
//       int data = v.get<int>();
//       f(k.c_str(), (void*)&data, (int)v.type(), nullptr);
//       break;
//     }
//     case nlohmann::json::value_t::number_unsigned:
//     {
//       unsigned int data = v.get<int>();
//       f(k.c_str(), (void*)&data, (int)v.type(), nullptr);
//       break;
//     }
//     case nlohmann::json::value_t::binary:
//     {
//       std::vector<uint8_t> bin = v.get<std::vector<uint8_t>>();
//       std::string s = gql::base64_encode(bin);
//       // f(k.c_str(), (void*)&(s[0]), (int)v.type(), nullptr);
//       break;
//     }
//     default:
//       break;
//     }
//   }
//   return ECode_Success;
// }

// int GStorageEngine::getNode(GGraphInstance* graph, const VertexID& nodeid, std::function<int(const char*, void*)> f)
// {
//   GVertexStmt vertex = graph->getVertexById(nodeid);
//   if (IS_INVALID_VERTEX(vertex.property())) return ECode_Success;
//   if (vertex.hasBinary()) {
//     for (auto& item : vertex.property())
//     {
//       if (item.type() == nlohmann::json::value_t::binary) {
//         nlohmann::json::binary_t bin = item;
//         item = "b64'" + gql::base64_encode(bin) + "'";
//       }
//     }
//   }
//   std::string out = vertex.property().dump();
//   out = std::regex_replace(out, std::regex("\"b64'([\\s\\S]+)'\"", std::regex_constants::ECMAScript), "b64'$1'");
//   f(out.c_str(), nullptr);
//   return ECode_Success;
// }

// std::vector<VertexID> GStorageEngine::getNodes(GGraphInstance* graph)
// {
//   auto vertexes = graph->getVertex(_txn);
//   std::vector<VertexID> ids;
//   for (auto& vertex : vertexes) {
//     VertexID vid(vertex.first);
//     ids.emplace_back(vid);
//   }
//   return ids;
// }

// int GStorageEngine::dropNode(GGraphInstance* graph, const VertexID& nodeid)
// {
//   graph->dropVertex(nodeid);
//   return ECode_Success;
// }

// int GStorageEngine::makeDirection(GGraphInstance* graph, const EdgeID& id, const VertexID& from, const VertexID& to, const char* name)
// {
//   // graph->updateEdge(id, name, "");
//   // make relation and then save it
//   graph->bind(id, from, to);
//   return ECode_Success;
// }
