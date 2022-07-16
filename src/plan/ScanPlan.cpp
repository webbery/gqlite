#include "plan/ScanPlan.h"
#include "base/lang/ASTNode.h"
#include "base/lang/QueryStmt.h"
#include "gqlite.h"
#include <filesystem>

GScanPlan::GScanPlan(GVirtualNetwork* network, GStorageEngine* store, GQueryStmt* stmt)
:GPlan(network, store){
  _graph = GetString(stmt->graph());
}

int GScanPlan::prepare()
{
  if (!std::filesystem::exists(_graph)) {
    return ECode_Graph_Not_Exist;
  }
  return ECode_Success;
}

int GScanPlan::execute(gqlite_callback cb) {
  _interrupt.store(false);
  _worker = std::thread(&GScanPlan::scan, this);
  return ECode_Success;
}

int GScanPlan::interrupt()
{
  _interrupt.store(true);
  _worker.join();
  return ECode_Success;
}

int GScanPlan::scan()
{
  while (_interrupt.load())
  {

  }
  return ECode_Success;
}
