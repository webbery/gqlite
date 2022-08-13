#pragma once
#include "Plan.h"
#include "Graph/GRAD.h"
#include "base/lang/AST.h"
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
  GScanPlan(GVirtualNetwork* network, GStorageEngine* store, GASTNode* condition, const std::string& graph = "");
  ~GScanPlan();

  virtual int prepare();
  virtual int execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>&);
  virtual int interrupt();

  std::vector<std::string> groups() { return _queries; }
private:
  int scan(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>& cb);

  void parseGroup(GASTNode* query);
  /**
   * parse condition node, retrieve simple condition/graph pattern or other kind of condition
   */
  void parseConditions(GASTNode* conditions);

  struct PatternVisitor {
    GraphPattern& _pattern;
    EntityNode* _node;

    PatternVisitor(GraphPattern& pattern, EntityNode* node) :_pattern(pattern), _node(node) {}

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
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
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

  GraphPattern _pattern;
  std::vector<std::string> _queries;
  std::string _graph;
};