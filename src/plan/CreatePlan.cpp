#include "plan/CreatePlan.h"
#include "gqlite.h"

GCreatePlan::GCreatePlan(GVirtualNetwork* vn): GPlan(vn) {}

int GCreatePlan::execute() {
  return ECode_Success;
}