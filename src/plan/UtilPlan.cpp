#include "plan/UtilPlan.h"
#include "gqlite.h"
#include "VirtualNetwork.h"
#include "StorageEngine.h"
#include "base/lang/AST.h"

GUtilPlan::GUtilPlan(GVirtualNetwork* vn, GStorageEngine* store, GASTNode* ast)
:GPlan(vn, store) {
  switch (ast->_nodetype)
  {
  case NodeType::CreationStatement:
  {
    _type = UtilType::Creation;
    CreationVisitor visitor(*this);
    std::list<NodeType> path;
    accept(ast, visitor, path);
  }
    break;
  default:
    break;
  }
}
int GUtilPlan::execute() {
  switch (_type)
  {
  case UtilType::Creation:
    if (!_store) return ECode_DISK_OPEN_FAIL;
    break;
  default:
    break;
  }
  return ECode_Success;
}
