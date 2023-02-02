#pragma once

struct GListNode;
class GQueryStmt {
public:
  GQueryStmt(GListNode* query, GListNode* graph, GListNode* conditions);
  ~GQueryStmt();

  GListNode* query()const { return _query; }
  GListNode* graph()const { return _graph; }
  GListNode* where()const { return _conditions; }
private:
  GListNode* _query;
  GListNode* _graph;
  GListNode* _conditions;

};