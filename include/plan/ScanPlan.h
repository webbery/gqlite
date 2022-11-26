#pragma once
#include "Plan.h"
#include "Graph/GRAD.h"
#include "base/lang/AST.h"
#include <string>
#include <atomic>
#include <thread>
#include <vector>
#include <stack>
#include "base/system/Observer.h"

class GQueryStmt;
struct GASTNode;
class GScanPlan: public GPlan {
  /**
   * Parse Stmt to Plan info list.
   * ScanPlan execute with plan info list.
   */
  struct PlanInfo {
    float cost;
    std::string _group;
  };
  using ScanPlans = std::vector<PlanInfo>;

  enum class QueryType {
    SimpleScan,     /**< scan database with/without simple condition */
    NNSearch,       /**< use KNN search */
    Match,          /**< subgraph match */
    Inference,      /**< bayes network inference */
  };

  enum class ScanState {
    Stop,
    Scanning,
    Pause
  };

public:
  GScanPlan(std::map<std::string, GVirtualNetwork*>& networks, GStorageEngine* store, GQueryStmt* stmt);
  GScanPlan(std::map<std::string, GVirtualNetwork*>& networks, GStorageEngine* store, GASTNode* condition, const std::string& graph = "");
  ~GScanPlan();

  void addObserver(IObserver* observer);
  virtual int prepare();
  virtual int execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>&);
  virtual int interrupt();

  void start();
  void pause();
  void goon();
  void stop();

  //std::vector<std::string> groups() { return _queries; }
private:
  int scan(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>& cb);
  // scan indexes
  int scan();

  void parseGroup(GASTNode* query);
  /**
   * parse condition node, retrieve simple condition/graph pattern or other kind of condition
   */
  void parseConditions(GASTNode* conditions);

  bool pauseExit(GStorageEngine::cursor& cursor, ScanPlans::iterator itr);

  bool stopExit();

  gkey_t getKey(KeyType type, mdbx::slice& slice);
  bool predict(gkey_t key, nlohmann::json& row);
  bool predict(const std::function<bool(const attribute_t&)>& op, const nlohmann::json& attr)const;
  // convert obj to display type
  void beautify(nlohmann::json& input);
  /**
   * @brief evaluate the order of simple scan's indexes
   * 
   * @return std::list<size_t> 
   */
  ScanPlans evaluate(const ScanPlans& props);

  struct PatternVisitor {
    QueryCondition& _where;
    EntityNode* _node;
    // and/or index
    long _index = 0;
    QueryType _qt =QueryType::SimpleScan;

    PatternVisitor(QueryCondition& where, EntityNode* node) :_where(where), _node(node) {}

    VisitFlow apply(GASTNode* stmt, std::list<NodeType>& path);
    VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GGQLExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GProperty* stmt, std::list<NodeType>& path);
    VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path);
    VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path) { return VisitFlow::Return; }
    VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path) { return VisitFlow::Return; }
    VisitFlow apply(GDumpStmt* stmt, std::list<NodeType>& path) { return VisitFlow::Return; }
    VisitFlow apply(GRemoveStmt* stmt, std::list<NodeType>& path) { return VisitFlow::Return; }
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path);
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path);
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path);
    VisitFlow apply(GObjectFunction* stmt, std::list<NodeType>& path) { return VisitFlow::Return; }
  };

  /**
   * @brief vertex query condition visitor
   */
  struct VertexJsonVisitor {
    GraphPattern& _pattern;
    EntityNode* _node;
    VertexJsonVisitor(GraphPattern& pattern, EntityNode* node) :_pattern(pattern), _node(node) {}

    VisitFlow apply(GASTNode* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GGQLExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GProperty* stmt, std::list<NodeType>& path);
    VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path);
    VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path) { return VisitFlow::Return; }
    VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path) { return VisitFlow::Return; }
    VisitFlow apply(GDumpStmt* stmt, std::list<NodeType>& path) { return VisitFlow::Return; }
    VisitFlow apply(GRemoveStmt* stmt, std::list<NodeType>& path) { return VisitFlow::Return; }
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path);
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path);
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path) { return VisitFlow::Return; }
    VisitFlow apply(GObjectFunction* stmt, std::list<NodeType>& path);
  };
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

  /**
   * QueryCondition contain graph pattern, node predictates
   */
  QueryCondition _where;
  /**
   * @brief estimate info of query table
   */
  ScanPlans _queries[(long)LogicalPredicate::Max];

  std::string _graph;
  std::string _group;
  
  std::vector<IObserver*> _observers;
  /**
   * scan status.
   */
  ScanState _state;

  struct ScanStackRecord {
    /**
   * this cursor will be reset when state is stop.
   * Or continue when go on.
   */
    GStorageEngine::cursor _cursor;
    ScanPlans::iterator _itr;
    LogicalPredicate _op;
  };
  ScanStackRecord _scanRecord;
  /**
   * save temp id that search from index
   */
  std::list< gkey_t > _resultSet;
};