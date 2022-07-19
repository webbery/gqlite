#pragma once
#include <mdbx.h++>
#include <list>
#include "json.hpp"
#include "operand/skey.h"
#include "IGraph.h"

#define BIN_FLAG  ":bin"
#define IS_INVALID_VERTEX(v) \
  (v.empty() || v.is_null() || v.size() == 0 || v.is_string())

mdbx::slice get(mdbx::txn_managed& txn, mdbx::map_handle& map, const std::string& key);
mdbx::slice get(mdbx::txn_managed& txn, mdbx::map_handle& map, int32_t key);
int put(mdbx::txn_managed& txn, mdbx::map_handle& map, const std::string& key, mdbx::slice value);
int put(mdbx::txn_managed& txn, mdbx::map_handle& map, int key, mdbx::slice value);

std::vector<uint8_t> serialize(const GraphProperty& property);
GraphProperty deserialize(const std::vector<uint8_t>& value);

mdbx::map_handle open_schema(mdbx::txn_managed& txn);

class GVertexProptertyFeature;
class IGist;
class GGraphInstance : public GGraphInterface {
public:
  // this schema save all database schema info, which key is graph name
  /**
   * [
   *   {
   *    name: graphname,
   *    index: ['property', ...]
   *   }
   * ]
   **/
  static mdbx::map_handle _schema;

  GGraphInstance(mdbx::txn_managed& txn, const char* name);
  ~GGraphInstance();

  // std::vector<Edge> getEdges();
  // std::vector<Edge> getEdges(mdbx::txn_managed& txn, VertexID vertex);
  std::vector<std::pair<VertexID, nlohmann::json>> getVertex(mdbx::txn_managed& txn);

  // int queryVertex(std::set<VertexID>& ids, const GConditions& pred);
  int queryEdge(const nlohmann::json& pred);
  // int query(const GMatchPattern& pattern);

    // std::vector<Vertex> getVertex(mdbx::txn_managed& txn, EdgeID edge);
  // GVertexStmt getVertexById(const std::string& id);

  int bind(const EdgeID& eid, const VertexID& from, const VertexID& to);

  int updateVertex(const VertexID& id, const std::vector<uint8_t>& data);
  int updateEdge(const EdgeID& id, const std::vector<uint8_t>& data);
  int updateLink();

  int dropVertex(const std::string& id);
  int dropEdge(const std::string& id);
  int dropLink();
  int drop();
  int finishUpdate(mdbx::txn_managed& txn);

  GraphValueType propertyType(const std::string& prop);

private:
  std::string vertexDBName(const char* name);
  std::string edgeDBName(const char* name);
  std::string vertexGraphName(const char* name);
  std::string edgeGraphName(const char* name);
  std::string schemaDBName(const char* name);
  int saveSchema();

  void UpsetMetaType(const std::string& name, GraphValueType vtype);
  GraphValueType GetMetaType(const std::string& name);
  bool isIndexExist(const std::string& name);
  GVertexProptertyFeature* getFeature(const char* property);

private:
  mdbx::txn_managed& _txn;
  // a key-value map save links of vertex and edges
  enum GraphKind { GK_Vertex, GK_Edge, GK_Size};
  mdbx::map_handle _graph[GK_Size];
  mdbx::map_handle _edges;
  mdbx::map_handle _vertexes;
  std::map<std::string, nlohmann::json> _updateVertexes;
  std::map<std::string, nlohmann::json> _updateEdges;
  struct Link {
    EdgeID eid;
    VertexID fid;
    VertexID tid;
  };
  std::vector< Link > _updateRelations;
  std::list<VertexID> _removeVertexes;
};