#include "VirtualEngine.h"
#include "gqlite.h"
#include "StorageEngine.h"
#include "base/lang/ASTNode.h"
#include "plan/Plan.h"
#include "plan/CreatePlan.h"
#include "VirtualNetwork.h"

uint32_t GVirtualEngine::_indx = 0;

uint32_t GVirtualEngine::GenerateIndex()
{
  return ++GVirtualEngine::_indx;
}

GVirtualEngine::GVirtualEngine()
: _errIndx(0)
, _errorCode(ECode_GQL_Parse_Fail)
, _cmdtype(GQL_Command_Size)
{
  _network = new GVirtualNetwork();
}

GVirtualEngine::~GVirtualEngine() {
  delete _network;
  if (_graph) {
    delete _graph;
    _graph = nullptr;
  }
}

void GVirtualEngine::cleanPlans(GPlan* plans) {
  if (plans) delete plans;
}

GPlan* GVirtualEngine::makePlans(GASTNode* ast) {
  if (ast == nullptr) return nullptr;
  GPlan* root = nullptr;
  switch (ast->_nodetype)
  {
  case NodeType::CreationStatement:
  {
    root = new GCreatePlan(_network, _storage, ast);
  }
    break;
  default:
    break;
  }
  for (size_t idx = 0; idx < ast->_size; ++idx) {
    makePlans(ast->_children + idx);
  }
  return root;
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