#include "VirtualEngine.h"
#include "base/gvm/Chunk.h"
#include "base/lang/AST.h"
#include "base/lang/BlockStmt.h"
#include "base/lang/VariableDecl.h"
#include "base/type.h"
#include "gqlite.h"
#include "StorageEngine.h"
#include "base/lang/ASTNode.h"
#include "plan/Plan.h"
#include "plan/RemovePlan.h"
#include "plan/UtilPlan.h"
#include "plan/UpsetPlan.h"
#include "plan/QueryPlan.h"
#include "plan/ScanPlan.h"
#include "VirtualNetwork.h"
#include "base/system/exception/CompileException.h"
#include "base/lang/visitor/VariantVisitor.h"

void init_result_info(gqlite_result& result, const std::vector<std::string>& info) {
  result.count = info.size();
  result.infos = (char**)malloc(result.count * sizeof(char*));
  for (size_t idx = 0; idx < result.count; ++idx) {
    result.infos[idx] = (char*)malloc(info[idx].size() + 1);
    memcpy(result.infos[idx], info[idx].data(), info[idx].size() + 1);
  }
  result.type = gqlite_result_type_cmd;
}

void release_result_info(gqlite_result& result) {
  for (size_t idx = 0; idx < result.count; ++idx) {
    free(result.infos[idx]);
  }
  free(result.infos);
}

const char* GVirtualEngine::GetErrorInfo(int code)
{
  switch (code)
  {
  case GQL_GRAMMAR_ARRAY_FAIL:
    return "array is not correct format";
  default:
    return nullptr;
  }
}

GVirtualEngine::GVirtualEngine(size_t memsize)
: _errIndx(0)
, _errorCode(ECode_GQL_Parse_Fail)
, _cmdtype(GQL_Command_Size)
, _result_callback(nullptr)
{
  //_network = new GVirtualNetwork(memsize);
  _gvm = new GVM();
}

GVirtualEngine::~GVirtualEngine() {
  for (auto& item: _networks)
  {
    delete item.second;
  }
  delete _gvm;
}

void GVirtualEngine::cleanPlans(PlanList* plans) {
  PlanList* cur = plans->_parent;
  while (cur != plans)
  {
#ifdef GQLITE_MULTI_THREAD
    if (cur->_threadable) {
      cur->_plan->interrupt();
    }
#endif
    delete cur->_plan;
    PlanList* temp = cur;
    cur = cur->_parent;
    delete temp;
  }
  delete cur;
}

GVirtualEngine::PlanList* GVirtualEngine::makePlans(GListNode* ast) {
  if (ast == nullptr) return nullptr;
  PlanVisitor visitor(_networks, _gvm, _storage, _result_callback, _handle);
  std::list<NodeType> ln;
  accept(ast, &visitor, ln);
  return visitor._plans;
}

int GVirtualEngine::executePlans(PlanList* plans) {
  int ret = ECode_Success;
  PlanList* cur = plans->_next;
  while (cur != plans)
  {
    ret = cur->_plan->prepare();
    if (ret != ECode_Success) return ret;
    ret = cur->_plan->execute(_gvm, [&](KeyType, const std::string& key, const std::string& value, int status) -> ExecuteStatus {
      return ExecuteStatus::Continue; });
    if (ret != ECode_Success) return ret;
    cur = cur->_next;
  }
  return ret;
}

int GVirtualEngine::execAST(GListNode* ast) {
  if (_errorCode < 0) return _errorCode;
  if (!ast) return ECode_Success;
  PlanList* plans = makePlans(ast);
  int ret = executePlans(plans);
  cleanPlans(plans);
  return ret;
}

int GVirtualEngine::execCommand(GListNode* ast)
{
  GGQLExpression* expr = (GGQLExpression * )ast->_value;
  switch (expr->type())
  {
  case GGQLExpression::CMDType::SHOW_GRAPH_DETAIL:
  {
    std::string graph = expr->params();
    auto jsn = _storage->getSchema();
    _gqlite_result result;
    init_result_info(result, { jsn.dump() });
    _result_callback(&result, _handle);
    release_result_info(result);
  }
    break;
  default:
    break;
  }
  return 0;
}

void GVirtualEngine::PlanVisitor::add(GPlan* plan, bool threadable)
{
  PlanList* next = new PlanList;
  PlanList* last = _plans->_parent;
  last->_next = next;
  _plans->_parent = next;
  next->_plan = plan;
  next->_next = _plans;
  next->_threadable = threadable;
  next->_parent = last;
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
  GPlan* plan = new GQueryPlan(_vn, _store, stmt, _cb, this->_handle);
  add(plan);
  return VisitFlow::Return;
}

VisitFlow GVirtualEngine::PlanVisitor::apply(GDumpStmt* stmt, std::list<NodeType>& path)
{
  GPlan* plan = new GUtilPlan(_vn, _store, stmt);
  add(plan);
  return VisitFlow::Return;
}

VisitFlow GVirtualEngine::PlanVisitor::apply(GRemoveStmt* stmt, std::list<NodeType>& path)
{
  GPlan* plan = new GRemovePlan(_vn, _store, stmt);
  add(plan);
  return VisitFlow::Return;
}

VisitFlow GVirtualEngine::PlanVisitor::apply(GObjectFunction* stmt, std::list<NodeType>& path) {
  return VisitFlow::Children;
}

VisitFlow GVirtualEngine::PlanVisitor::apply(GLambdaExpression* stmt, std::list<NodeType>& path) {
  ByteCodeVisitor visitor(_gvm);
  std::list<NodeType> ln;
  GListNode* block = stmt->block();
  accept(block, &visitor, ln);
  // disassembleChunk("lambda", *visitor._currentChunk);
  _plans->_parent->_plan->addChunk(visitor._currentChunk);
  _plans->_parent->_plan->addCompiler(visitor._compiler);
  // generate byte code
  return VisitFlow::SkipCurrent;
}

VisitFlow GVirtualEngine::PlanVisitor::apply(GReturnStmt* stmt, std::list<NodeType>& path) {
  return VisitFlow::Children;
}

void GVirtualEngine::ByteCodeVisitor::emit(uint8_t byte) {
  if (!_currentChunk) {
    _currentChunk = new Chunk;
  }
  _currentChunk->_code.push_back(byte);
}

VisitFlow GVirtualEngine::ByteCodeVisitor::apply(GBinaryExpression* stmt, std::list<NodeType>& path) {
  accept(stmt->left(), this, path);
  accept(stmt->right(), this, path);
  emit(stmt->getOperator());
  return VisitFlow::SkipCurrent;
}

VisitFlow GVirtualEngine::ByteCodeVisitor::apply(GLiteral* stmt, std::list<NodeType>& path) {
  Value v;
  if (GetLiteral(stmt, v)) {
    emit((uint8_t)OpCode::OP_CONSTANT, addConstant(*_currentChunk, v));
  }
  return VisitFlow::SkipCurrent;
}

VisitFlow GVirtualEngine::ByteCodeVisitor::apply(GReturnStmt* stmt, std::list<NodeType>& path) {
  accept(stmt->expr(), this, path);
  emit(stmt->getOperator());
  return VisitFlow::SkipCurrent;
}

VisitFlow GVirtualEngine::ByteCodeVisitor::apply(GBlockStmt* stmt, std::list<NodeType>& path) {
  _compiler->_scopeDepth += 1;
  // accept(stmt->expr(), *this, path);
  // emit(stmt->getOperator());
  return VisitFlow::SkipCurrent;
}

VisitFlow GVirtualEngine::ByteCodeVisitor::apply(GVariableDecl* stmt, std::list<NodeType>& path) {
  GVariantVisitor visitor;
  accept(stmt->value(), &visitor, path);
  if (_compiler->_scopeDepth == 0) {
    // global variant
    Value value = visitor.getVariant();
    if (_gvm->setGlobalVariant(stmt->name(), value) == ECode_Compile_Warn_Var_Exist) {
      throw GCompileException();
    }
    return VisitFlow::SkipCurrent;
  }
  // local variant

  return VisitFlow::SkipCurrent;
}