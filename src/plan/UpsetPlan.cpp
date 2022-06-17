#include "plan/UpsetPlan.h"
#include "StorageEngine.h"
#include "VirtualNetwork.h"

GUpsetPlan::GUpsetPlan(GVirtualNetwork* vn, GStorageEngine* store, GASTNode* ast)
:GPlan(vn, store) {}

int GUpsetPlan::execute() {
  GPlan* plan = _left;
  while (plan)
  {
    _network->addNode(nullptr);
    plan = plan->left();
  }
  
  return 0;
}
