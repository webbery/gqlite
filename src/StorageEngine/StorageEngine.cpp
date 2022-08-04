#include "StorageEngine.h"
#include <regex>
#include <stdio.h>
#include <zstd.h>
#include <iostream>
#include <atomic>

#ifdef WIN32
#pragma comment(lib, BINARY_DIR "/" CMAKE_INTDIR "/zstd_static.lib")
#endif

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

  mdbx::slice get(mdbx::txn_managed& txn, mdbx::map_handle& map, uint64_t key)
  {
    mdbx::slice k(&key, sizeof(uint64_t));
    mdbx::slice absent;
    return txn.get(map, k, absent);
  }

  int put(mdbx::txn_managed& txn, mdbx::map_handle& map, const std::string& key, mdbx::slice value)
  {
    mdbx::slice k(key.data(), key.size());
    txn.put(map, k, value, mdbx::upsert);
    return 0;
  }

  int put(mdbx::txn_managed& txn, mdbx::map_handle& map, uint64_t key, mdbx::slice value)
  {
    mdbx::slice k(&key, sizeof(uint64_t));
    txn.put(map, k, value, mdbx::upsert);
    return 0;
  }

  int del(mdbx::txn_managed& txn, mdbx::map_handle& map, uint64_t key) {
    mdbx::slice k(&key, sizeof(uint64_t));
    if (txn.erase(map, k)) return 0;
    return -1;
  }

  int del(mdbx::txn_managed& txn, mdbx::map_handle& map, const std::string& key) {
    mdbx::slice k(key.data(), key.size());
    if (txn.erase(map, k)) return 0;
    return -1;
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

  _env = env_managed(option.directory + filename, create_param, operator_param);
  int ret = startTrans();
  mdbx::map_handle handle = openSchema();
  thread_local auto id = std::this_thread::get_id();
  mdbx::slice data = ::get(_txns[id], handle, SCHEMA_BASIC);
  if (data.size()) {
    std::vector<uint8_t> v(data.byte_ptr(), data.byte_ptr() + data.size());
    _schema = nlohmann::json::from_cbor(v);
  }
  _schema[SCHEMA_GRAPH_NAME] = filename;
  initMap(option);
  // _env.close_map(handle);
  return ret;
}

void GStorageEngine::close()
{
  thread_local auto id = std::this_thread::get_id();
  if (_txns.count(id) && _txns[id].is_readwrite()) {
    if (!_schema.empty()) {
      mdbx::map_handle handle = openSchema();
      std::vector<uint8_t> v = nlohmann::json::to_cbor(_schema);
      mdbx::slice data(v.data(), v.size());
      ::put(_txns[id], handle, SCHEMA_BASIC, data);
      // _env.close_map(handle);
    }
    _txns[id].commit();
  }
  _txns.clear();
}

mdbx::map_handle GStorageEngine::openSchema() {
  mdbx::map_handle schema;
  thread_local auto id = std::this_thread::get_id();
  GRAPH_EXCEPTION_CATCH(schema = _txns[id].create_map(DB_SCHEMA, mdbx::key_mode::usual, mdbx::value_mode::single));
  return schema;
}

void GStorageEngine::initMap(StoreOption option)
{
  if (!isMapExist(MAP_BASIC)) {
    MapInfo info;
    info.key_type = KeyType::Uninitialize;
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

bool GStorageEngine::compare(const std::string& left, const std::string& right) const
{
  if (left == right) return true;
  if (left[left.size()] == '\0' && left.substr(0, left.size() - 1) == right) return true;
  if (right[right.size()] == '\0' && right.substr(0, right.size() - 1) == left) return true;
  return false;
}

bool GStorageEngine::isMapExist(const std::string& prop) {
  if (_schema.empty()) return false;
  const auto& props = _schema[SCHEMA_CLASS];
  for (auto itr = props.begin(); itr != props.end(); ++itr) {
    if (compare((*itr)[SCHEMA_CLASS_NAME], prop)) return true;
  }
  return false;
}

void GStorageEngine::tryInitKeyType(const std::string& prop, KeyType type)
{
  static std::atomic_bool init = false;
  if (init) return;
  auto& props = _schema[SCHEMA_CLASS];
  for (auto itr = props.begin(); itr != props.end(); ++itr) {
    if (compare((*itr)[SCHEMA_CLASS_NAME], prop)) {
      init.store(true);
      uint8_t info = (*itr)[SCHEMA_CLASS_INFO];
      MapInfo* p = (MapInfo*)&info;
      if (p->key_type == KeyType::Uninitialize) p->key_type = type;
      (*itr)[SCHEMA_CLASS_INFO] = info;
      return;
    }
  }
}

nlohmann::json GStorageEngine::getProp(const std::string& prop)
{
  const auto& props = _schema[SCHEMA_CLASS];
  for (auto itr = props.begin(); itr != props.end(); ++itr) {
    if (compare((*itr)[SCHEMA_CLASS_NAME], prop)) return *itr;
  }
  assert(false);
}

mdbx::map_handle GStorageEngine::getOrCreateHandle(const std::string& prop, mdbx::key_mode mode) {
  thread_local auto id = std::this_thread::get_id();
  if (_mHandles[id].count(prop) == 0) {
    mdbx::map_handle propMap;
    GRAPH_EXCEPTION_CATCH(propMap = _txns[id].create_map(prop, mode, mdbx::value_mode::single));
    _mHandles[id][prop] = propMap;
  }
  return _mHandles[id][prop];
}

int GStorageEngine::write(const std::string& prop, const std::string& key, void* value, size_t len) {
  assert(isMapExist(prop));
  tryInitKeyType(prop, KeyType::Byte);
  mdbx::slice data(value, len);
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::usual);
  thread_local auto id = std::this_thread::get_id();
  ::put(_txns[id], handle, key, data);
  return ECode_Success;
}

int GStorageEngine::read(const std::string& prop, const std::string& key, std::string& value) {
  assert(isMapExist(prop));
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::usual);
  thread_local auto id = std::this_thread::get_id();
  mdbx::slice data = ::get(_txns[id], handle, key);
  if (data.empty()) return ECode_DATUM_Not_Exist;
  value.assign((char*)data.data(), data.size());
  return ECode_Success;
}

int GStorageEngine::del(const std::string& mapname, const std::string& key)
{
  assert(isMapExist(mapname));
  auto handle = getOrCreateHandle(mapname, mdbx::key_mode::usual);
  thread_local auto id = std::this_thread::get_id();
  if (::del(_txns[id], handle, key)) return ECode_Fail;
  return ECode_Success;
}

int GStorageEngine::del(const std::string& mapname, uint64_t key)
{
  assert(isMapExist(mapname));
  auto handle = getOrCreateHandle(mapname, mdbx::key_mode::ordinal);
  thread_local auto id = std::this_thread::get_id();
  if (::del(_txns[id], handle, key)) return ECode_Fail;
  return ECode_Success;
}

int GStorageEngine::write(const std::string& prop, uint64_t key, void* value, size_t len) {
  assert(isMapExist(prop));
  tryInitKeyType(prop, KeyType::Integer);
  mdbx::slice data(value, len);
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::ordinal);
  // compress
  size_t inSize = ZSTD_CStreamInSize();
  thread_local auto id = std::this_thread::get_id();
  ::put(_txns[id], handle, key, data);
  //printf("write: %d\n", key);
  return ECode_Success;

}
int GStorageEngine::read(const std::string& prop, uint64_t key, std::string& value) {
  assert(isMapExist(prop));
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::ordinal);
  thread_local auto id = std::this_thread::get_id();
  mdbx::slice data = ::get(_txns[id], handle, key);
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
  MapInfo info;
  size_t n = sizeof(MapInfo);
  std::memcpy(&info, &c, sizeof(MapInfo));
  mdbx::map_handle handle;
  if (info.key_type == KeyType::Integer) {
    handle = getOrCreateHandle(prop, mdbx::key_mode::ordinal);
  }
  else {
    handle = getOrCreateHandle(prop, mdbx::key_mode::usual);
  }
  thread_local auto id = std::this_thread::get_id();
  return _txns[id].open_cursor(handle);
}

int GStorageEngine::startTrans(ReadWriteOption opt) {
  thread_local auto id = std::this_thread::get_id();
  mdbx::txn_managed txn;
  if (_txns.count(id) == 0) {
    if (opt == ReadWriteOption::read_only) {
      txn = _env.start_read();
    }
    else {
      txn = _env.start_write();
    }
    if (!txn) return ECODE_NULL_PTR;
    _txns[id] = std::move(txn);
  }
  return ECode_Success;
}

int GStorageEngine::finishTrans() {
  thread_local auto id = std::this_thread::get_id();
  if (_txns.count(id) == 0) return ECode_TRANSTION_Not_Exist;
  _txns[id].commit();
  return ECode_Success;
}

KeyType GStorageEngine::getKeyType(const std::string& m) const
{
  const auto& props = _schema[SCHEMA_CLASS];
  for (auto itr = props.begin(); itr != props.end(); ++itr) {
    if (compare((*itr)[SCHEMA_CLASS_NAME], m)) {
      uint8_t info = (*itr)[SCHEMA_CLASS_INFO];
      MapInfo* p = (MapInfo*)&info;
      return p->key_type;
    }
  }
  return KeyType::Uninitialize;
}
