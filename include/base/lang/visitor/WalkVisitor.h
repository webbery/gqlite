#pragma once
#include "base/lang/visitor/IVisitor.h"
#include "Graph/GRAD.h"
#include "base/lang/WalkDeclaration.h"

enum class QueryType;
class GWalkVisitor: public GVisitor {
public:
  enum class WalkType {
    EdgeQuery,    /*< query edge like vertex query*/
    GraphMatch,   /*< subgraph match */
    PathQuery,    /*< query path */
  };

  GWalkVisitor(GraphPattern* pattern);

  VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path);

  VisitFlow apply(GWalkDeclaration*, std::list<NodeType>&);

  VisitFlow apply(GObjectFunction*, std::list<NodeType>&);

  WalkType walkType() const { return _walkType; }
private:
  void makeEdgeCondition(GWalkDeclaration::Order order, EntityNode* start, EntityNode* end, bool direction);

  EntityNode* makeNodeCondition(const std::string& str);

private:
  /**
   * A nano graph that describe the query graph. This graph will be use for matching orperator.
   */
  GraphPattern* _graph;

  WalkType _walkType;
};