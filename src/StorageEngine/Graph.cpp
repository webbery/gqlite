#include "Graph.h"
#include "Error.h"
#include <set>
#include "Type/Binary.h"
#include "gqlite.h"

#define GRAPH_EXCEPTION_CATCH(expr) try{\
  expr;\
}catch(std::exception& err) {printf("exception %s [%d]: %s\n", __FILE__, __LINE__, err.what());}
#define DB_SCHEMA   "gqlite_schema"

#define PROPERTY_MAX_COUNT  "cnt"
#define PROPERTY_NAME       "n"
#define INDEX_NAME          "i"

namespace {
}

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

std::vector<uint8_t> serialize(const GraphProperty& property)
{
  nlohmann::json js;
  std::vector<uint32_t> vui({ property._vertexCount, property._edgeCount });
  js[PROPERTY_MAX_COUNT] = vui;
  js[PROPERTY_NAME] = property._name;
  js[INDEX_NAME] = property._indexes;
  return nlohmann::json::to_cbor(js);
}

GraphProperty deserialize(const std::vector<uint8_t>& value)
{
  nlohmann::json js = nlohmann::json::from_cbor(value);
  GraphProperty property;
  property._vertexCount = js[PROPERTY_MAX_COUNT][0];
  property._edgeCount = js[PROPERTY_MAX_COUNT][1];
  property._name = js[PROPERTY_NAME];
  if (js.contains(INDEX_NAME)) property._indexes = js[INDEX_NAME].get<std::vector<std::string>>();
  return property;
}

mdbx::map_handle GGraphInstance::_schema;
mdbx::map_handle open_schema(mdbx::txn_managed& txn)
{
  mdbx::map_handle schema;
  GRAPH_EXCEPTION_CATCH(schema = txn.create_map(DB_SCHEMA, mdbx::key_mode::usual, mdbx::value_mode::single));
  return schema;
}


GGraphInstance::GGraphInstance(mdbx::txn_managed& txn, const char* name)
  :_txn(txn)
{
  std::string vName = vertexDBName(name);
  std::string eName = edgeDBName(name);
  GRAPH_EXCEPTION_CATCH(_vertexes = txn.create_map(vName, mdbx::key_mode::usual, mdbx::value_mode::single));
  GRAPH_EXCEPTION_CATCH(_edges = txn.create_map(eName, mdbx::key_mode::usual, mdbx::value_mode::single));
  GRAPH_EXCEPTION_CATCH(_graph[GK_Vertex] = txn.create_map(vertexGraphName(name),
    mdbx::key_mode::usual, mdbx::value_mode::single));
  GRAPH_EXCEPTION_CATCH(_graph[GK_Edge] = txn.create_map(edgeGraphName(name),
    mdbx::key_mode::usual, mdbx::value_mode::single));
  if (_schema.dbi == 0) {
    GRAPH_EXCEPTION_CATCH(_schema = txn.create_map(DB_SCHEMA, mdbx::key_mode::usual, mdbx::value_mode::single));
    mdbx::slice data = get(txn, _schema, name);
    if (data.size()) {
      std::vector<uint8_t> v(data.byte_ptr(), data.byte_ptr() + data.size());
      _property = deserialize(v);
    }
    else {
      _property._name = name;
      saveSchema();
    }
  }
}

GGraphInstance::~GGraphInstance()
{
}

// std::vector<Edge> GGraph::getEdges(mdbx::txn_managed& txn, VertexID vertex)
// {
//   mdbx::slice data = get(txn, _graph[GK_Vertex], vertex);
//   std::vector<Edge> vEdges;
//   if (!data.size()) return vEdges;
//   return vEdges;
// }

std::vector<std::pair<VertexID, nlohmann::json>> GGraphInstance::getVertex(mdbx::txn_managed& txn)
{
  std::vector<std::pair<VertexID, nlohmann::json>> result;
  using namespace mdbx;
  cursor_managed cursor = txn.open_cursor(_vertexes);
  if (cursor.on_last()) return result;
  cursor.to_first();
  while (true)
  {
    cursor::move_result item = cursor.current();
    if (!item.done) break;
    std::string key((char*)item.key.data(), item.key.size());
    if (!item.value.empty()) {
      auto data = nlohmann::json::from_cbor((const char*)item.value.data(), (const char*)item.value.data() + item.value.size() - 1);
      result.push_back({ key, data });
    }
    else {
      result.push_back({ key, "" });
    }
    if (cursor.on_last()) break;
    cursor.to_next();
  }
  return result;
}

/*int GGraphInstance::queryVertex(std::set<VertexID>& ids, const GConditions& preds)
{
  const GConditions* cur = &preds;
  while (cur) {
    std::shared_ptr<GPredition> pred = cur->_preds;
    std::set<VertexID> lastResult;
    while (pred) {
      GVertexProptertyFeature* feature = getFeature(pred->_indx.c_str());
      if (feature == nullptr) {
        return ECode_GQL_Index_Not_Exist;
      }
      std::set<VertexID> temp;
      using namespace mdbx;
      mdbx::cursor_managed cursor;
      int status = feature->get_cursor(_txn, pred->_value, pred->_type, cursor);
      if (status == ECode_GQL_Vertex_Not_Exist || status == ECode_Fail) {
        pred = pred->_next;
        continue;
      }
      while (true) {
        cursor::move_result item = cursor.current();
        if (!item.done) break;
        switch (pred->_type) {
        case PropertyKind::Number:
        {
          uint64_t value = *(uint64_t*)item.key.byte_ptr();
          uint64_t right = std::any_cast<uint64_t>(pred->_value);
          if (get_internal_predicate<uint64_t>(pred->_fn)(value, right)) {
            VertexID id((char*)item.value.byte_ptr(), item.value.size());
            temp.insert(id);
          }
        }
        break;
        case PropertyKind::String:
        {
          std::string left((char*)item.key.byte_ptr(), item.key.size());
          std::string right = std::any_cast<std::string>(pred->_value);
          if (get_internal_predicate<std::string>(pred->_fn)(left, right)) {
            VertexID id((char*)item.value.byte_ptr(), item.value.size());
            temp.insert(id);
          }
        }
        break;
        }
        if (cursor.on_last()) break;
        cursor.to_next();
      }
      pred = pred->_next;
      if (!lastResult.size()) {
        lastResult = temp;
        break;
      }
      std::set<VertexID> out;
      if (cur->_isAnd) {
        std::set_intersection(lastResult.begin(), lastResult.end(), temp.begin(), temp.end(), std::inserter(out, out.begin()));
      }
      else {
        std::set_union(lastResult.begin(), lastResult.end(), temp.begin(), temp.end(), std::inserter(out, out.begin()));
      }
      lastResult = out;
    }
    cur = cur->_next.get();
  }
  return ECode_Success;
}*/

int GGraphInstance::queryEdge(const nlohmann::json& pred)
{
  return ECode_Success;
}

// int GGraph::query(const GMatchPattern& pattern)
// {
//   return ECode_Success;
// }

// GVertexStmt GGraphInstance::getVertexById(const std::string& id)
// {
//   GVertexStmt vertex;
//   mdbx::slice data = get(_txn, _vertexes, id);
//   if (data.empty()) return vertex;
//   vertex.property() = nlohmann::json::from_cbor(data.byte_ptr(), data.byte_ptr() + data.size() - 1);
//   size_t offset = data.size();
//   mdbx::byte extra = *(data.byte_ptr() + offset - 1);
//   // vertex.setBinaryFlag(extra & EXTERN_BINARY_BIT);
//   vertex.setBinaryFlag(true);
//   return vertex;
// }

int GGraphInstance::bind(const EdgeID& eid, const VertexID& from, const VertexID& to)
{
  this->_updateRelations.push_back({ eid, from, to });
  return 0;
}

int GGraphInstance::updateVertex(const VertexID& id, const std::vector<uint8_t>& data)
{
  mdbx::slice bin(data.data(), data.size());
  return put(_txn, _vertexes, id, bin);
}

int GGraphInstance::updateEdge(const EdgeID& id, const std::vector<uint8_t>& data)
{
  return 0;
}

int GGraphInstance::dropVertex(const std::string& id)
{
  _removeVertexes.push_back(id);
  return 0;
}

int GGraphInstance::drop() {
  _txn.drop_map(_edges);
  _txn.drop_map(_schema);
  for (int idx = 0; idx < GK_Size; ++idx) {
    if (_graph[idx].dbi) _txn.drop_map(_graph[idx]);
  }
  return 0;
}

int GGraphInstance::finishUpdate(mdbx::txn_managed& txn)
{
  // delete vertexes
  std::for_each(_removeVertexes.begin(), _removeVertexes.end(), [&](auto item)
    {
      mdbx::slice k(item.data(), item.size());
      if (!txn.erase(_vertexes, k)) {
        printf("remove vertex[%s] fail\n", item.c_str());
      }
    }
  );
  _removeVertexes.clear();
  return 0;
}

bool GGraphInstance::isIndexExist(const std::string& name)
{
  for (std::string& index: _property._indexes) {
    if (index == name) return true;
  }
  return false;
}

GVertexProptertyFeature* GGraphInstance::getFeature(const char* property) {
  return nullptr;
}
int GGraphInstance::saveSchema()
{
  std::vector<uint8_t> v = serialize(_property);
  auto data = mdbx::slice(v.data(), v.data() + v.size());
  return put(_txn, _schema, _property._name, data);
}

GraphValueType GGraphInstance::propertyType(const std::string& prop)
{
  auto itr = _property._types.find(prop);
  if (itr == _property._types.end()) return GraphValueType::Undefined;
  return itr->second;
}

std::string GGraphInstance::vertexDBName(const char* name)
{
  return std::string("V:") + name;
}

std::string GGraphInstance::edgeDBName(const char* name)
{
  return std::string("E:") + name;
}

std::string GGraphInstance::vertexGraphName(const char* name)
{
  return std::string("gv:") + name;
}

std::string GGraphInstance::edgeGraphName(const char* name)
{
  return std::string("ge:") + name;
}
