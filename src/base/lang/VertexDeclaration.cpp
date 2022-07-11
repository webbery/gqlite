#include "base/lang/VertexDeclaration.h"
#include "base/lang/ASTNode.h"

GVertexDeclaration::GVertexDeclaration(GASTNode* key, GASTNode* nodes)
:_head(nodes)
,_key(key)
{}

GVertexDeclaration::~GVertexDeclaration() {
  if (_head) {
    FreeAst(_head);
  }
  FreeAst(_key);
}