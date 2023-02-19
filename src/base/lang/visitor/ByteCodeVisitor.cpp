#include "base/lang/visitor/ByteCodeVisitor.h"
#include "base/gvm/Chunk.h"
#include "base/gvm/Compiler.h"
#include "base/gvm/GVM.h"
#include "base/lang/GQLExpression.h"
#include "base/lang/visitor/IVisitor.h"
#include "base/lang/visitor/VariantVisitor.h"
#include "base/system/exception/CompileException.h"
#include "base/type.h"

GByteCodeVisitor::GByteCodeVisitor(GVM* gvm, Compiler* compiler)
: _gvm(gvm), _return(false) {
  if (compiler == nullptr) {
    _compiler = new Compiler(nullptr);
  }
  else {
    _compiler = new Compiler(compiler);
  }
}

VisitFlow GByteCodeVisitor::apply(GReturnStmt* stmt, std::list<NodeType>& path) {
  accept(stmt->expr(), this, path);
  _compiler->emit(stmt->getOperator());
  _return = true;
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GBlockStmt* stmt, std::list<NodeType>& path) {
  _compiler->_scopeDepth += 1;
  accept(stmt->block(), this, path);
  _compiler->_scopeDepth -= 1;
  // set to gvm
  while (_compiler->_count > 0 && _compiler->_variant[_compiler->_count - 1]._depth > _compiler->_scopeDepth) {
    _compiler->emit((uint8_t)OpCode::OP_POP);
    _compiler->_count--;
  }
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GBinaryExpression* stmt, std::list<NodeType>& path) {
  accept(stmt->left(), this, path);
  accept(stmt->right(), this, path);
  _compiler->emit(stmt->getOperator());
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GAssignStmt* stmt, std::list<NodeType>& path) {
  accept(stmt->decl(), this, path);
  GVariantVisitor visitor(_gvm, _compiler);
  accept(stmt->value(), &visitor, path);
  if (!stmt->isDecl()) {
    _compiler->namedVariant(stmt->name(), true);
  }
  //Value value = visitor.getVariant();
  //_gvm->declareLocalVariant(_compiler, stmt->name(), value);
  //_compiler->namedVariant(stmt->name(), true);
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GLiteral* stmt, std::list<NodeType>& path) {
  Value v;
  if (GetLiteral(stmt, v)) {
    _compiler->emit((uint8_t)OpCode::OP_CONSTANT, addConstant(_compiler->currentChunk(), v));
  }
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GObjectFunction* func, std::list<NodeType>& path) {
  Value name(func->name());
  int idxArgCount = func->size();
  if (idxArgCount >= std::numeric_limits<uint8_t>::max()) {
    throw GCompileException("argument count more than 255");
  }
  if (_gvm->isGlobalExist(func->name())) {
    const Value& v = _gvm->getGlobalVariant(func->name());
    _compiler->emit((uint8_t)OpCode::OP_CONSTANT, addConstant(_compiler->currentChunk(), v.Get<NativeObj*>()));
  }
  else {
    FunctionObj* obj = new FunctionObj;
    obj->name = func->name();
    obj->arity = idxArgCount;
    obj->chunk = _compiler->currentChunk();
    _compiler->emit((uint8_t)OpCode::OP_CONSTANT, addConstant(_compiler->currentChunk(), obj));
  }

  if (idxArgCount) {
    for (auto itr = func->params_begin(); itr != func->params_end(); ++itr) {
      GVariantVisitor visitor(_gvm, _compiler);
      std::list<NodeType> ln;
      accept(*itr, &visitor, ln);
    }
  }
  _compiler->emit((uint8_t)OpCode::OP_CALL, idxArgCount);
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GVariableDecl* var, std::list<NodeType>& path) {
  if (_compiler->_scopeDepth != 0) {
    const std::string& name = var->name();
    _compiler->declLocal(name);
  }
  return VisitFlow::SkipCurrent;
}
