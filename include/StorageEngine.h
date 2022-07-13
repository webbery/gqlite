#pragma once
#include "gqlite.h"
#include <mdbx.h++>
#include "Feature/Feature.h"
#include <map>
#include <functional>

#define SCHEMA_CLASS            "cls"
#define SCHEMA_CLASS_INFO       "info"
#define SCHEMA_CLASS_NAME       "name"
#define SCHEMA_CLASS_DEFAULT    "default"

enum class ClassType : uint8_t {
    Undefined,
    String,
    Number,
    Custom
};

struct alignas(8) ClassInfo {
  uint8_t       key_type : 1;    /**<  0 - interger, 1 - byte; */
  ClassType  value_type : 5;  /**< */ 
  uint8_t       reserved : 2;
};

class GStorageEngine {
public:
    GStorageEngine();
    ~GStorageEngine();

    /** Open a graph instance with file.
     * A file is a graph instance.
     * After open success, its shcema is loaded.
     * The schema contains:
     *   all classes, all attribute's info(such as name, value type) of class
     * When reading the data, it will use schema and convert to GRAD graph
     * @param filename database filename
     */
    int open(const char* filename);

    void close();

    /** Add new class to graph schema.
     * It will create a new map with `prop`, which key type is info.key_type.
     * Example:
     *   In MovieLens, it has class like `MOVIE`, `ACTOR`, `USER` etc.
     * @param clazz class name
     * @param info  class infomation.
     */
    void addClass(const std::string& clazz, ClassInfo info);

    /** Record an node/edge infomation to disk
     * For example:
     *   <MOVIE, Star Trek, 3884>
     *   `MOVIE` is class, movie name `Star Trek` is key, 3884 is value.
     * @param clazz class name
     * @param key   the key of node/edge
     * @param value the attribute of node/edge
     */
    int write(const std::string& clazz, const std::string& key, void* value, size_t len);
    int read(const std::string& clazz, const std::string& key, std::string& value);

    int write(const std::string& clazz, uint64_t key, void* value, size_t len);
    int read(const std::string& clazz, uint64_t key, std::string& value);

    typedef mdbx::cursor_managed  cursor;
    cursor getCursor(const std::string& clazz);

    void registGraphFeature(GGraphInstance*, GVertexProptertyFeature* feature);

    /** 
     * Get the schema of current graph instance.
     * Schema is a json which format as follows:
     *   
     */
    nlohmann::json getSchema() { return _schema; }

    int startTrans();

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

private:
    bool isClassExist(const std::string& prop);
    nlohmann::json getProp(const std::string& prop);
    mdbx::map_handle getOrCreateHandle(const std::string& prop, mdbx::key_mode mode);
    /*
     * @brief schema is used to record the graph's infomation
     */
    mdbx::map_handle openSchema();

private:
    mdbx::env_managed _env;
    mdbx::txn_managed _txn;
    std::map<std::string, mdbx::map_handle> _mHandle;
    /**
     * schema: {
     *   prop: [ {name: 'xx', type: undefined/str/number} ]
     * }
     */
    nlohmann::json _schema;
};