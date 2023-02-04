#pragma once

struct GListNode;

class GBlockStmt {
public:
  GBlockStmt(GListNode* node):_block(node) {}
  ~GBlockStmt();

private:
  GListNode* _block;
};