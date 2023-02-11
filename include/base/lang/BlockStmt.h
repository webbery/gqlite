#pragma once

struct GListNode;

class GBlockStmt {
public:
  GBlockStmt(GListNode* node):_block(node) {}
  ~GBlockStmt();

  GListNode* block() const { return _block; }
private:
  GListNode* _block;
};