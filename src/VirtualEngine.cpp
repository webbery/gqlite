#include "VirtualEngine.h"
#include "gqlite.h"
#include "StorageEngine.h"
#include "base/lang/ASTNode.h"
#include "plan/UtilPlan.h"
#include "plan/UpsetPlan.h"
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
{
  _network = new GVirtualNetwork(memsize);
}

GVirtualEngine::~GVirtualEngine() {
  delete _network;
}

void GVirtualEngine::cleanPlans(GPlan* plans) {
  if (plans) delete plans;
}

GPlan* GVirtualEngine::makePlans(GASTNode* ast) {
  if (ast == nullptr) return nullptr;
  PlanVisitor visitor(_network, _storage);
  std::list<NodeType> ln;
  accept(ast, visitor, ln);
  return visitor._plan;
}

int GVirtualEngine::executePlans(GPlan* plans) {
  int ret = ECode_Success;
  if (plans) {
    ret = plans->execute();
  }
  return ret;
}

int GVirtualEngine::execAST(GASTNode* ast) {
  if (!ast) return ECode_Success;
  GPlan* plans = makePlans(ast);
  int ret = executePlans(plans);
  cleanPlans(plans);
  return ret;
}

VisitFlow GVirtualEngine::PlanVisitor::apply(GCreateStmt* stmt, std::list<NodeType>& path)
{
  _plan = new GUtilPlan(_vn, _store, stmt);
  return VisitFlow::Children;
}
