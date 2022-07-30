#pragma once
#include "Plan.h"
#include "Graph/GRAD.h"
#include <string>
#include <atomic>
#include <thread>
#include <vector>

class GQueryStmt;
struct GASTNode;
class GScanPlan: public GPlan {
  enum class QueryType {
    SimpleScan,     /**< scan database with/without simple condition */
    NNSearch,       /**< use KNN search */
    Match,          /**< subgraph match */
    Inference,      /**< bayes network inference */
  };
public:
  GScanPlan(GVirtualNetwork* network, GStorageEngine* store, GQueryStmt* stmt);
  virtual int prepare();
  virtual int execute(gqlite_callback);
  virtual int interrupt();

private:
  int scan(gqlite_callback cb = nullptr);

  void parseQuery(GASTNode* query);
private:
  /**
   * @brief A query type that mark difference query.
   *        SimpleScan: Only scan database. Index may be used.
   *        NNSearch: A HNSW network will be created for search.
   *        Match: Virtual network will be created when scan database.
   *        Inference: query probability of event/hidden variant etc.
   */
  QueryType _queryType;

  std::atomic_bool _interrupt;
  /**
   * worker thread will scan graph instance. If it find out a node or
   * edge that match the graph pattern, it will put it to virtual network.
   */
  std::thread _worker;

  GraphPattern _pattern;
  std::vector<std::string> _queries;
  std::string _graph;
};