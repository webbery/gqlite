#pragma once
#include "../type.h"
#include <stddef.h>

struct GASTNode {
  enum NodeType _nodetype;
  void* _value;
  struct GASTNode* _children;
  size_t _size;
};

GASTNode* NewAst(enum NodeType type, void* value, GASTNode* children, size_t size);
void FreeAst(GASTNode* root);
void DumpAst(GASTNode* root, int level = 0);

GASTNode* LoadAST();

GASTNode* ListJoin(GASTNode* first, GASTNode* second);
