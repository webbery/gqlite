#pragma once

struct GListNode;
class GWhereClause {
public:
  GWhereClause(GListNode* match);

private:
  GListNode* _match;
};