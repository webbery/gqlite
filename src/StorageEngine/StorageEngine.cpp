#include "StorageEngine.h"
#include <regex>
#include <zstd.h>
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

int GStorageEngine::open(const char* filename, StoreOption option) {
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
  initMap(option);
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

void GStorageEngine::initMap(StoreOption option)
{
  if (!isMapExist(MAP_BASIC)) {
    MapInfo info = { 0 };
    info.key_type = 1;
    info.value_type = ClassType::String;
    addMap(MAP_BASIC, info);
  }
  _schema[SCHEMA_GLOBAL][GLOBAL_COMPRESS_LEVEL] = option.compress;
}

void GStorageEngine::addMap(const std::string& prop, MapInfo type) {
  if (!isMapExist(prop)) {
    nlohmann::json info;
    info[SCHEMA_CLASS_NAME] = prop;
    uint8_t value = *(uint8_t*)&type;
    info[SCHEMA_CLASS_INFO] = value;
    _schema[SCHEMA_CLASS].push_back(info);
  }
}

bool GStorageEngine::isMapExist(const std::string& prop) {
  if (_schema.empty()) return false;
  const auto& props = _schema[SCHEMA_CLASS];
  for (auto itr = props.begin(); itr != props.end(); ++itr) {
    if ((*itr)[SCHEMA_CLASS_NAME] == prop) return true;
  }
  return false;
}

nlohmann::json GStorageEngine::getProp(const std::string& prop)
{
  const auto& props = _schema[SCHEMA_CLASS];
  for (auto itr = props.begin(); itr != props.end(); ++itr) {
    if ((*itr)[SCHEMA_CLASS_NAME] == prop) return *itr;
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
  // assert(isMapExist(prop));
  mdbx::slice data(value, len);
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::usual);
  ::put(_txn, handle, key, data);
  return ECode_Success;
}

int GStorageEngine::read(const std::string& prop, const std::string& key, std::string& value) {
  // assert(isMapExist(prop));
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::usual);
  mdbx::slice data = ::get(_txn, handle, key);
  if (data.empty()) return ECode_DATUM_Not_Exist;
  value.assign((char*)data.data(), data.size());
  return ECode_Success;
}

int GStorageEngine::write(const std::string& prop, uint64_t key, void* value, size_t len) {
  // assert(isMapExist(prop));
  mdbx::slice data(value, len);
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::ordinal);
  // compress
  // size_t inSize = ZSTD_CStreamInSize();
  ::put(_txn, handle, key, data);
  return ECode_Success;

}
int GStorageEngine::read(const std::string& prop, uint64_t key, std::string& value) {
  assert(isMapExist(prop));
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::ordinal);
  mdbx::slice data = ::get(_txn, handle, key);
  if (data.empty()) return ECode_DATUM_Not_Exist;
  value.assign((char*)data.data(), data.size());
  return ECode_Success;
}

GStorageEngine::cursor GStorageEngine::getCursor(const std::string& prop)
{
  assert(isMapExist(prop));
  mdbx::key_mode mode = mdbx::key_mode::ordinal;
  auto pp = getProp(prop);
  auto c = (uint8_t)pp[SCHEMA_CLASS_INFO];
  MapInfo info = { 0 };
  size_t n = sizeof(MapInfo);
  std::memcpy(&info, &c, sizeof(MapInfo));
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

int GStorageEngine::startTrans() {
  _txn = _env.start_write();
  if (!_txn) return ECODE_NULL_PTR;
  return ECode_Success;
}

int GStorageEngine::finishTrans() {
  _txn.commit();
  return ECode_Success;
}
