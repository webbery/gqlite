#include "base/lang/VertexDeclaration.h"
#include "base/lang/ASTNode.h"

GVertexDeclaration::GVertexDeclaration(GListNode* key, GListNode* nodes)
:_head(nodes)
,_key(key)
{}

GVertexDeclaration::~GVertexDeclaration() {
  if (_head) {
    FreeNode(_head);
  }
  FreeNode(_key);
}