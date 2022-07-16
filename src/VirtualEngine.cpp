#include "VirtualEngine.h"
#include "gqlite.h"
#include "StorageEngine.h"
#include "base/lang/ASTNode.h"
#include "plan/UtilPlan.h"
#include "plan/UpsetPlan.h"
#include "plan/QueryPlan.h"
#include "plan/ScanPlan.h"
#include "VirtualNetwork.h"

uint32_t GVirtualEngine::_indx = 0;

uint32_t GVirtualEngine::GenerateIndex()
{
  return ++GVirtualEngine::_indx;
}

GVirtualEngine::GVirtualEngine(size_t memsize)
: _errIndx(0)
, _errorCode(ECode_GQL_Parse_Fail)
, _cmdtype(GQL_Command_Size)
, _result_callback(nullptr)
{
  _network = new GVirtualNetwork(memsize);
}

GVirtualEngine::~GVirtualEngine() {
  delete _network;
}

void GVirtualEngine::cleanPlans(PlanList* plans) {
  PlanList* cur = plans->_parent;
  while (cur != plans)
  {
    if (cur->_threadable) {
      cur->_plan->interrupt();
    }
    delete cur->_plan;
    PlanList* temp = cur;
    cur = cur->_parent;
    delete temp;
  }
}

GVirtualEngine::PlanList* GVirtualEngine::makePlans(GASTNode* ast) {
  if (ast == nullptr) return nullptr;
  PlanVisitor visitor(_network, _storage);
  std::list<NodeType> ln;
  accept(ast, visitor, ln);
  return visitor._plans;
}

int GVirtualEngine::executePlans(PlanList* plans) {
  int ret = ECode_Success;
  PlanList* cur = plans->_next;
  while (cur != plans)
  {
    cur->_plan->prepare();
    if (ret != ECode_Success) return ret;
    ret = cur->_plan->execute(_result_callback);
    if (ret != ECode_Success) return ret;
    cur = cur->_next;
  }
  return ret;
}

int GVirtualEngine::execAST(GASTNode* ast) {
  if (!ast) return ECode_Success;
  PlanList* plans = makePlans(ast);
  int ret = executePlans(plans);
  cleanPlans(plans);
  return ret;
}

void GVirtualEngine::PlanVisitor::add(GPlan* plan, bool threadable)
{
  PlanList* next = new PlanList;
  next->_plan = plan;
  next->_next = _plans;
  next->_threadable = threadable;
  PlanList* pl = _plans->_parent;
  next->_parent = pl;
  pl->_next = next;
}

VisitFlow GVirtualEngine::PlanVisitor::apply(GUpsetStmt* stmt, std::list<NodeType>& path) {
  GPlan* plan = new GUpsetPlan(_vn, _store, stmt);
  add(plan);
  return VisitFlow::Children;
}
VisitFlow GVirtualEngine::PlanVisitor::apply(GCreateStmt* stmt, std::list<NodeType>& path)
{
  GPlan* plan = new GUtilPlan(_vn, _store, stmt);
  add(plan);
  return VisitFlow::Children;
}

VisitFlow GVirtualEngine::PlanVisitor::apply(GDropStmt* stmt, std::list<NodeType>& path) {
  GPlan* plan = new GUtilPlan(_vn, _store, stmt);
  add(plan);
  return VisitFlow::Return;
}

VisitFlow GVirtualEngine::PlanVisitor::apply(GQueryStmt* stmt, std::list<NodeType>& path)
{
  GPlan* scan = new GScanPlan(_vn, _store, stmt);
  add(scan, true);
  //GPlan* plan = new GQueryStmt();
  return VisitFlow::Return;
}
