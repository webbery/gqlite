#include "StorageEngine.h"
#include <regex>
#include <stdio.h>
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

GStorageEngine::GStorageEngine()
  :_cdict(nullptr)
  ,_ddict(nullptr)
, _cctx(nullptr) {
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
  initDict(option.compress);
  return ret;
}

bool GStorageEngine::isOpen()
{
  if (_schema.empty()) return false;
  return true;
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
  releaseDict();
}

mdbx::map_handle GStorageEngine::openSchema() {
  mdbx::map_handle schema;
  thread_local auto id = std::this_thread::get_id();
  GRAPH_EXCEPTION_CATCH(schema = _txns[id].create_map(DB_SCHEMA, mdbx::key_mode::usual, mdbx::value_mode::single));
  return schema;
}

void GStorageEngine::initMap(StoreOption option)
{
  addMap(MAP_BASIC, KeyType::Uninitialize);
}

void GStorageEngine::initDict(int compressLvl)
{
  if (compressLvl > 3 || compressLvl <= 0) compressLvl = 1;
  if (_schema[SCHEMA_GLOBAL][GLOBAL_COMPRESS_LEVEL].empty()) {
    _schema[SCHEMA_GLOBAL][GLOBAL_COMPRESS_LEVEL] = compressLvl;
  }
  else {
    compressLvl = _schema[SCHEMA_GLOBAL][GLOBAL_COMPRESS_LEVEL];
  }
  static std::map<int, size_t> dictSize = {
    {1, 32 * 1024},
    {2, 64 * 1024},
    {3, 1024 * 1024},
  };
  if (_schema[SCHEMA_GLOBAL].count(GLOBAL_COMPRESS_DICT)) {
    std::vector<uint8_t> bin = _schema[SCHEMA_GLOBAL][GLOBAL_COMPRESS_DICT];
    if (bin.size() >= dictSize[compressLvl] - 5)
      _ddict = ZSTD_createDDict(bin.data(), bin.size());
  }
  else {
    char* buffer = (char*)malloc(dictSize[compressLvl]);
    _cdict = ZSTD_createCDict(buffer, dictSize[compressLvl], compressLvl);
    free(buffer);
  }
  _cctx = ZSTD_createCCtx();
}

void GStorageEngine::releaseDict()
{
  if (_cctx) ZSTD_freeCCtx(_cctx);
  if (_cdict) ZSTD_freeCDict(_cdict);
  if (_ddict) ZSTD_freeDDict(_ddict);
}

void GStorageEngine::addMap(const std::string& prop, KeyType type) {
  if (!isMapExist(prop)) {
    _schema[SCHEMA_CLASS][prop][SCHEMA_CLASS_KEY] = type;
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
  if (props.is_null()) return false;
  return props.count(prop) != 0;
}

void GStorageEngine::tryInitKeyType(const std::string& prop, KeyType type)
{
  static bool init = false;
  if (init) return;
  init = true;
  _schema[SCHEMA_CLASS][prop][SCHEMA_CLASS_KEY] = type;
}

void GStorageEngine::tryInitAttributeType(nlohmann::json& attributes, const std::string& attr, const nlohmann::json& value)
{
  if (attributes.count(attr) == 0) {
    if (attributes.size() > 255) {
      // throw error?
      throw std::exception();
    }
    uint8_t index = attributes.size();
    switch ((nlohmann::json::value_t)value) {
    case nlohmann::json::value_t::object:
      if (value.count("_obj_type")) {
        attributes[attr] = std::pair((AttributeKind)value["_obj_type"], index);
      }
      else {
        attributes[attr] = std::pair(AttributeKind::String, index);
      }
      break;
    case nlohmann::json::value_t::array:
    case nlohmann::json::value_t::string:
      attributes[attr] = std::pair(AttributeKind::String, index);
      break;
    case nlohmann::json::value_t::number_integer:
    case nlohmann::json::value_t::number_unsigned:
    case nlohmann::json::value_t::number_float:
      attributes[attr] = std::pair(AttributeKind::Number, index);
      break;
    case nlohmann::json::value_t::binary:
      attributes[attr] = std::pair(AttributeKind::Binary, index);
      break;
    default:
      break;
    }
  }
}

void GStorageEngine::appendValue(uint8_t attrIndex, AttributeKind kind, const nlohmann::json& value, std::string& data)
{
  data.push_back(attrIndex);
  switch (kind)
  {
  case AttributeKind::String:
    data.append(value.dump());
    break;
  case AttributeKind::Binary:
    break;
  case AttributeKind::Number:
  {
    double v = value;
    char buf[sizeof(double)] = { 0 };
    std::memcpy(buf, &v, sizeof(double));
    data.append(buf, sizeof(double));
  }
  break;
  case AttributeKind::Datetime:
    break;
  default:
    break;
  }
}

nlohmann::json GStorageEngine::getProp(const std::string& prop)
{
  const auto& props = _schema[SCHEMA_CLASS];
  return props[prop];
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
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::usual);
  thread_local auto id = std::this_thread::get_id();

  size_t cSize = len;
#ifdef _ENABLE_COMPRESS_
  void* buffer = malloc(2 * len);
  if (_cdict) ZSTD_compress_usingCDict(_cctx, buffer, 2 * len, value, len, _cdict);
  //else if (_ddict) ZSTD_compress_usingDDict(_cctx, buffer,)
#else
  void* buffer = value;
#endif
  mdbx::slice data(buffer, cSize);
  ::put(_txns[id], handle, key, data);
#ifdef _ENABLE_COMPRESS_
  free(buffer);
#endif
  return ECode_Success;
}

int GStorageEngine::write(const std::string& mapname, const std::string& key, const nlohmann::json& value)
{
  tryInitKeyType(mapname, KeyType::Byte);
  auto& attributes = _schema[SCHEMA_CLASS][mapname][SCHEMA_CLASS_VALUE];
  // convert json to gqlite store format: attribute index(max value is 256), [if binary, here put size]data, index, data, ...
  for (auto itr = value.begin(), end = value.end(); itr!=end;++itr)
  {
    const std::string& attr = itr.key();
    auto& value = itr.value();
    tryInitAttributeType(attributes, attr, value);
    //std::pair<AttributeKind, uint8_t> info = attributes[attr];
    //appendValue(info.second, info.first, value, data);
  }
  std::string data = value.dump();
  return write(mapname, key, data.data(), data.size());
}

int GStorageEngine::write(const std::string& mapname, uint64_t key, const nlohmann::json& value)
{
  tryInitKeyType(mapname, KeyType::Integer);
  auto& attributes = _schema[SCHEMA_CLASS][mapname][SCHEMA_CLASS_VALUE];
  for (auto itr = value.begin(), end = value.end(); itr != end; ++itr) {
    const std::string& attr = itr.key();
    auto& value = itr.value();
    std::string sv = value.dump();
    tryInitAttributeType(attributes, attr, value);
    //std::pair<AttributeKind, uint8_t> info = attributes[attr];
    //appendValue(info.second, info.first, value, data);
  }
  std::string data = value.dump();
  return write(mapname, key, data.data(), data.size());
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

int GStorageEngine::read(const std::string& mapname, const std::string& key, nlohmann::json& value)
{
  return ECode_Success;
}

int GStorageEngine::read(const std::string& mapname, uint64_t key, nlohmann::json& value)
{
  return ECode_Success;
}

int GStorageEngine::parse(const std::string& data, nlohmann::json& value)
{

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
  auto handle = getOrCreateHandle(prop, mdbx::key_mode::ordinal);
  // compress
#ifdef _ENABLE_COMPRESS_
  void* buffer = malloc(2 * len);
  size_t cSize = ZSTD_compress_usingCDict(_cctx, buffer, 2* len, value, len, _cdict);
#else
  void* buffer = value;
  size_t cSize = len;
#endif
  thread_local auto id = std::this_thread::get_id();
  mdbx::slice data(buffer, cSize);
  ::put(_txns[id], handle, key, data);
#ifdef _ENABLE_COMPRESS_
  free(buffer);
#endif
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
  auto type = (KeyType)pp[SCHEMA_CLASS_KEY];
  mdbx::map_handle handle;
  if (type == KeyType::Integer) {
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
  if (_schema[SCHEMA_CLASS][m].empty()) return KeyType::Uninitialize;
  return _schema[SCHEMA_CLASS][m][SCHEMA_CLASS_KEY];
}
