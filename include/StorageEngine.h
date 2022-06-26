#pragma once
#include "gqlite.h"
#include <mdbx.h++>
#include "Feature/Feature.h"
#include <map>
#include <functional>

enum class PropertyType : uint8_t {
    Undefined,
    String,
    Number,
    Custom
};

struct alignas(8) PropertyInfo {
  uint8_t       key_type : 1;    // 0 - interger, 1 - byte;
  PropertyType  value_type : 5;  // 
  uint8_t       reserved : 2;
};

class GStorageEngine {
public:
    GStorageEngine();
    ~GStorageEngine();

    int open(const char* filename);
    void close();

    void addProp(const std::string& prop, PropertyInfo info);

    int write(const std::string& prop, const std::string& key, void* value, size_t len);
    int read(const std::string& prop, const std::string& key, std::string& value);

    int write(const std::string& prop, uint64_t key, void* value, size_t len);
    int read(const std::string& prop, uint64_t key, std::string& value);

    typedef mdbx::cursor_managed  cursor;
    cursor getCursor(const std::string& prop);

    void registGraphFeature(GGraphInstance*, GVertexProptertyFeature* feature);

    nlohmann::json getSchema() { return _schema; }
    /**
     * add attribute of schema
     */
    void addAttribute(const std::string& key, const std::string& value);

    int startTrans();

    int finishTrans();

    int rollbackTrans();

    // int group();

    /**
     * generate edge id or vertex id
     * @param type 0-vertex, 1-edge
     */
    // int generateID(GGraphInstance* graph, char type, char*& id);

    // int finishUpdate(GGraphInstance* graph);

    // int getNode(GGraphInstance* graph, const VertexID& nodeid, std::function<int(const char*, void*, int, void*)>);
    // int getNode(GGraphInstance* graph, const VertexID& nodeid, std::function<int(const char*, void*)>);

    // std::vector<VertexID> getNodes(GGraphInstance* graph);

    // int dropNode(GGraphInstance* graph, const VertexID& nodeid);

    // int dropRelationship();

    /*
     *
     */
    // int startNode(const char* graph, const VertexID& nodeid);

    // int forward();

    // int backward();

    //template<typename T>
    //int makeDirection(GGraph* graph, const VertexID& from, const VertexID& to, const char* name, T&& value){}

    // int makeDirection(GGraphInstance* graph, const EdgeID& id, const VertexID& from, const VertexID& to, const char* name);

    int injectCostFunc();
    int injectNodeUpdateFunc();

private:
    bool isPropExist(const std::string& prop);
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