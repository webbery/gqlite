#include "plan/RemovePlan.h"
#include <stdio.h>
#include "base/lang/RemoveStmt.h"
#include "plan/ScanPlan.h"

GRemovePlan::GRemovePlan(GVirtualNetwork* network, GStorageEngine* store, GRemoveStmt* stmt)
  :GPlan(network, store)
{
  _scan = new GScanPlan(network, _store, stmt->node());
}

GRemovePlan::~GRemovePlan()
{
  delete _scan;
}

int GRemovePlan::execute(gqlite_callback)
{
  printf("remove\n");
  return 0;
}
