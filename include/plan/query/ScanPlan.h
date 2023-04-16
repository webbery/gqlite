#pragma once
#include "plan/Plan.h"
#include "Graph/GRAD.h"
#include "base/lang/AST.h"
#include <string>
#include <atomic>
#include <thread>
#include <vector>
#include <stack>
#include "base/lang/visitor/IVisitor.h"
#include "base/system/Observer.h"

class GQueryStmt;
struct GListNode;
class GScanPlan: public GPlan {
protected:
  /**
   * Parse Stmt to Plan info list.
   * ScanPlan execute with plan info list.
   */
  struct PlanInfo {
    float cost;
    std::string _group;
  };
  using ScanPlans = std::vector<PlanInfo>;

  enum class ScanState {
    Stop,
    Scanning,
    Pause
  };

public:
  GScanPlan(GContext* context, GQueryStmt* stmt);
  GScanPlan(GContext* context, GListNode* condition, const std::string& graph = "");
  ~GScanPlan();

  void addObserver(IObserver* observer);
  virtual int prepare();
  virtual int execute(GVM* gvm, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>&);
  virtual int interrupt();

  void start();
  void pause();
  void goon();
  void stop();

  //std::vector<std::string> groups() { return _queries; }
protected:
  int scan(const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>& cb);
  // scan indexes
  int scan();

  void parseGroup(GListNode* query);
  /**
   * parse condition node, retrieve simple condition/graph pattern or other kind of condition
   */
  void parseConditions(GListNode* conditions);

  bool pauseExit(GStorageEngine::cursor& cursor, ScanPlans::iterator itr);

  bool stopExit();

  gql::key_t getKey(KeyType type, mdbx::slice& slice);
  bool predict(KeyType type, gql::key_t key, nlohmann::json& row);
  bool predictEdge(gql::key_t key, nlohmann::json& row);
  bool predictVertex(gql::key_t key, nlohmann::json& row);
  bool predict(const std::function<bool(const attribute_t&)>& op, const nlohmann::json& attr)const;

  void initQueryGroups(const std::string& group);

  /**
   * @brief evaluate the order of simple scan's indexes
   * 
   * @return std::list<size_t> 
   */
  ScanPlans evaluate(const ScanPlans& props);

  struct PatternVisitor : public GVisitor {
    QueryCondition& _where;
    // and/or index
    long _index[2] = {0, 1};
    bool _isAnd = true;

    QueryType _qt =QueryType::SimpleScan;


    /**
     * attributes that will use to compare
     */
    std::vector<attr_node_t> _attrs[2];

    PatternVisitor(QueryCondition& where) :_where(where) {}

    VisitFlow apply(GProperty* stmt, std::list<NodeType>& path);
    VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path);
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path);
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path);
    VisitFlow apply(GWalkDeclaration* stmt, std::list<NodeType>& path);
    VisitFlow apply(GLambdaExpression* stmt, std::list<NodeType>& path);
  };

protected:
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
  // simple tag if no condition
  bool _scanAll;
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
  std::list< gql::key_t > _resultSet;
  GVM* _gvm;
};