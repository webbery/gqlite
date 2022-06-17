#pragma once
#include "gqlite.h"
#include <mdbx.h++>
#include "Graph.h"
#include "Type/Hash.h"
#include "Type/Binary.h"
#include "Type/Datetime.h"
#include "Type/Integer.h"
#include "Type/Real.h"
#include "Type/Text.h"
#include "Feature/Feature.h"
#include <map>
#include <functional>

enum class PropertyType {
    Undefined,
    String,
    Number
};

class GStorageEngine {
public:
    GStorageEngine();
    ~GStorageEngine();

    int open(const char* filename);
    void close();

    void addProp(const std::string& prop, PropertyType type = PropertyType::Undefined);

    int write(const std::string& prop, const std::string& key, void* value, size_t len);
    int read(const std::string& prop, const std::string& key, std::string& value);

    int write(const std::string& prop, uint64_t key, void* value, size_t len);
    int read(const std::string& prop, uint64_t key, std::string& value);

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