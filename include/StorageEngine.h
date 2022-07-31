#pragma once
#include "gqlite.h"
#include <mdbx.h++>
#include <map>
#include <functional>
#include <thread>
#include <json.hpp>

#define SCHEMA_GRAPH_NAME       "name"
#define SCHEMA_CLASS            "cls"
#define SCHEMA_CLASS_INFO       "info"
#define SCHEMA_CLASS_NAME       "name"
#define MAP_BASIC               "__basic"

#define SCHEMA_GLOBAL           "__global"
#define GLOBAL_COMPRESS_LEVEL   "__lvl"

#define GQL_VERSION             "0.0.1"

enum class ClassType : uint8_t {
    Undefined,
    String,
    Number,
    Custom
};

enum class KeyType : uint8_t {
  Uninitialize,
  Integer,
  Byte,
};

struct alignas(8) MapInfo {
  KeyType       key_type : 2;    /**<  0 - uninitialize, 1 - interger, 2 - byte; */
  ClassType     value_type : 4;  /**< */ 
  uint8_t       reserved : 2;
};

struct StoreOption {
  uint8_t       compress;   /**< compress level: 0~ */
  std::string   directory;  /**< directory of graph file */
};

enum class ReadWriteOption {
  read_only,
  write_only,
  read_write
};

class GStorageEngine {
public:
    GStorageEngine();
    ~GStorageEngine();

    /** 
     * @brief Open a graph instance with file.
     *        A file is a graph instance.
     *        After open success, its schema is loaded.
     *        The schema contains:
     *          all classes, all attributes info(such as name, value type) of class
     *        For the first time open, it will create some map handle as follows:
     *        1. `basic`. Include schema
     *        2. `node`. Include all node with format: <node_t, {attr_id: value, ...}>
     *        3. `edge`. Inlcude all edge with format: <edge_t, {attr_id: value, ...}>
     *        4. `link`. Relationship of node and edge: <node_t, [edge_t, edge_t, ...]>
     * @param filename database filename
     */
    int open(const char* filename, StoreOption option);

    void close();
    void close(mdbx::txn_managed& txn);

    /** Add new class to graph schema.
     * It will create a new map with `prop`, which key type is info.key_type.
     * Example:
     *   In MovieLens, it has class like `MOVIE`, `ACTOR`, `USER` etc.
     * @param mapname new map name
     * @param info  map information.
     */
    void addMap(const std::string& mapname, MapInfo info);

    /** 
     * @brief Record an node/edge information to disk
     *        For example:
     *          <MOVIE, Star Trek, 3884>
     *        `MOVIE` is class, movie name `Star Trek` is key, 3884 is value.
     * @param mapname class name
     * @param key   the key of node/edge
     * @param value the attribute of node/edge
     */
    int write(const std::string& mapname, const std::string& key, void* value, size_t len);
    int read(const std::string& mapname, const std::string& key, std::string& value);

    int write(const std::string& mapname, uint64_t key, void* value, size_t len);
    int read(const std::string& mapname, uint64_t key, std::string& value);

    typedef mdbx::cursor_managed  cursor;
    cursor getCursor(const std::string& mapname);

    /** 
     * Get the schema of current graph instance.
     * Schema is a json which format as follows:
     *   {
     *     name: graph_name(filename)
     *     version: 0.0.1(example),
     *     class(which values are map name): [Movie, Actor, ...],
     *     Movie(class detail): [{Score: value type}, {Title: string}, {WebID:...}, ...],
     *     Actor(class detail): [{nodes: current count}, {edges: current count}, ],
     *   }
     */
    nlohmann::json& getSchema() { return _schema; }

    int startTrans(ReadWriteOption opt = ReadWriteOption::read_write);

    int finishTrans();

    int rollbackTrans();

    // int group();

    /**
     * generate edge id or vertex id
     * @param type 0-vertex, 1-edge
     */
    // int generateID(GGraphInstance* graph, char type, char*& id);

    int injectCostFunc();
    int injectNodeUpdateFunc();

    /**
     * @brief get map's key type
     */
    KeyType getKeyType(const std::string& m) const;
private:
  /**
   * @brief because json lib return string without '\0', so write this method
   */
  bool compare(const std::string& left, const std::string& right) const;
    /**
     * @brief check map(prop) is exist or not.
     */
    bool isMapExist(const std::string& prop);
    /**
     * @brief check map's key type is init or not. If not, set it with `type`.
     */
    void tryInitKeyType(const std::string& prop, KeyType type);
    nlohmann::json getProp(const std::string& prop);
    mdbx::map_handle getOrCreateHandle(const std::string& prop, mdbx::key_mode mode);
    /*
     * @brief schema is used to record the graph's information
     */
    mdbx::map_handle openSchema();

    void initMap(StoreOption );

private:
    mdbx::env_managed _env;
    std::map<std::thread::id, mdbx::txn_managed> _txns;
    using handle_t = std::map<std::string, mdbx::map_handle>;
    std::map<std::thread::id, handle_t> _mHandles;
    /**
     * schema: {
     *   prop: [ {name: 'xx', type: undefined/str/number} ]
     * }
     */
    nlohmann::json _schema;
};