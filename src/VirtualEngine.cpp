#include "VirtualEngine.h"
#include "gqlite.h"
#include "StorageEngine.h"
#include "base/lang/ASTNode.h"
#include "plan/Plan.h"
#include "plan/CreatePlan.h"

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
}

GVirtualEngine::~GVirtualEngine() {
  if (_graph) {
    delete _graph;
    _graph = nullptr;
  }
}

void GVirtualEngine::cleanPlans(GPlan* plans) {
  if (!plans) return;
  delete plans;
}

GPlan* GVirtualEngine::makePlans(GASTNode* ast) {
  if (ast == nullptr) return nullptr;
  GPlan* root = nullptr;
  switch (ast->_nodetype)
  {
  case NodeType::CreationStatement:
  {
    root = new GCreatePlan(_network);
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

int GVirtualEngine::executePlans(GPlan*) {

  return ECode_Success;
}

int GVirtualEngine::execAST(GASTNode* ast) {
  if (!ast) return ECode_Success;
  GPlan* plans = makePlans(ast);
  int ret = executePlans(plans);
  cleanPlans(plans);
  return ret;
}