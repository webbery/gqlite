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

class GStorageEngine {
public:
    GStorageEngine();
    ~GStorageEngine();

    int create(const char* filename);

    int openGraph(const char* name);

    void registGraphFeature(GGraphInstance*, GVertexProptertyFeature* feature);

    GGraphInstance* getGraph(const char* name = nullptr);
    std::vector<std::string> getGraphs();

    int dropGraph(GGraphInstance* pGraph);
    int closeGraph(GGraphInstance* pGraph);

    int startTrans();

    int finishTrans();

    int rollbackTrans();

    int group();

    /**
     * generate edge id or vertex id
     * @param type 0-vertex, 1-edge
     */
    int generateID(GGraphInstance* graph, char type, char*& id);

    int finishUpdate(GGraphInstance* graph);

    int getNode(GGraphInstance* graph, const VertexID& nodeid, std::function<int(const char*, void*, int, void*)>);
    int getNode(GGraphInstance* graph, const VertexID& nodeid, std::function<int(const char*, void*)>);

    std::vector<VertexID> getNodes(GGraphInstance* graph);

    int dropNode(GGraphInstance* graph, const VertexID& nodeid);

    int dropRelationship();

    /*
     *
     */
    int startNode(const char* graph, const VertexID& nodeid);

    int forward();

    int backward();

    //template<typename T>
    //int makeDirection(GGraph* graph, const VertexID& from, const VertexID& to, const char* name, T&& value){}

    int makeDirection(GGraphInstance* graph, const EdgeID& id, const VertexID& from, const VertexID& to, const char* name);

    int injectCostFunc();
    int injectNodeUpdateFunc();

private:
    int openGraph(const char* name, GGraphInstance*& pGraph);
    /*
     * @brief schema is used to record all the graph's name
     */
    void schema();
private:
    mdbx::env_managed _env;
    mdbx::txn_managed _txn;
    std::map<std::string, GGraphInstance*> _mHandle;
    std::string _usedgraph;
};