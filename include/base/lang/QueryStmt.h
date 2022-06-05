#pragma once

struct GASTNode;
class GQueryStmt {
public:
  GQueryStmt(GASTNode* query, GASTNode* graph, GASTNode* conditions);

  GASTNode* query()const { return _query; }
  GASTNode* graph()const { return _graph; }
  GASTNode* where()const { return _conditions; }
private:
  GASTNode* _query;
  GASTNode* _graph;
  GASTNode* _conditions;
};