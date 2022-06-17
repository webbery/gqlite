#include "plan/CreatePlan.h"
#include "gqlite.h"
#include "base/lang/ASTNode.h"
#include "StorageEngine.h"

GCreatePlan::GCreatePlan(GVirtualNetwork* vn, GStorageEngine* store, GASTNode* ast): GPlan(vn, store) {
  GTypeTraits<NodeType::CreationStatement>::type* ptr = reinterpret_cast<GTypeTraits<NodeType::CreationStatement>::type*>(ast->_value);
  _name = ptr->name();
  GASTNode* list = ptr->indexes();
  size_t cnt = 1;
  while (list) {
    GASTNode* node = (GASTNode*)list->_value;
    _indexes.emplace_back(GetString(node));
    list = list->_children;
  }
}

int GCreatePlan::execute() {
  _store->addAttribute("graph", _name);
  return ECode_Success;
}
