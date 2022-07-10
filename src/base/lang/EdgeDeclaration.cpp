#include "base/lang/EdgeDeclaration.h"
#include "base/lang/ASTNode.h"

GEdgeDeclaration::GEdgeDeclaration(GASTNode* from, GASTNode* to, GASTNode* link)
:_from(from)
,_to(to)
, _link(link) {}

GEdgeDeclaration::~GEdgeDeclaration()
{
  if (_from != _to) {
    FreeAst(_to);
  }
  FreeAst(_from);
  FreeAst(_link);
}
