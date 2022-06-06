#include "base/lang/EdgeDeclaration.h"

GEdgeDeclaration::GEdgeDeclaration(GASTNode* from, GASTNode* to, GASTNode* link)
:_from(from)
,_to(to)
,_link(link) {}