#include "base/lang/visitor/ByteCodeVisitor.h"
#include "base/gvm/Chunk.h"
#include "base/gvm/GVM.h"
#include "base/lang/GQLExpression.h"
#include "base/lang/visitor/VariantVisitor.h"
#include "base/type.h"

GByteCodeVisitor::GByteCodeVisitor(GVM* gvm, Compiler* compiler)
: _gvm(gvm), _compiler(compiler){
  if (_compiler == nullptr) {
    _compiler = new Compiler;
  }
}

VisitFlow GByteCodeVisitor::apply(GReturnStmt* stmt, std::list<NodeType>& path) {
  accept(stmt->expr(), this, path);
  emit(stmt->getOperator());
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GBlockStmt* stmt, std::list<NodeType>& path) {
  _compiler->_scopeDepth += 1;
  accept(stmt->block(), this, path);
  _compiler->_scopeDepth -= 1;
  // set to gvm
  while (_compiler->_count > 0 && _compiler->_variant[_compiler->_count - 1]._depth > _compiler->_scopeDepth) {
    emit((uint8_t)OpCode::OP_POP);
    _compiler->_count--;
  }
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GBinaryExpression* stmt, std::list<NodeType>& path) {
  accept(stmt->left(), this, path);
  accept(stmt->right(), this, path);
  emit(stmt->getOperator());
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GAssignStmt* stmt, std::list<NodeType>& path) {
  accept(stmt->decl(), this, path);
  GVariantVisitor visitor(_gvm, _compiler);
  accept(stmt->value(), &visitor, path);
  Value value = visitor.getVariant();
  _gvm->declareLocalVariant(_compiler, stmt->name(), value);
  namedVariant(stmt->name(), true);
  
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GLiteral* stmt, std::list<NodeType>& path) {
  Value v;
  if (GetLiteral(stmt, v)) {
    emit((uint8_t)OpCode::OP_CONSTANT, addConstant(_compiler->currentChunk(), v));
  }
  return VisitFlow::SkipCurrent;
}

VisitFlow GByteCodeVisitor::apply(GObjectFunction* func, std::list<NodeType>& path) {
  if (func->isIntrinsic()) {
    Value name(func->name());
    int idxName = addConstant(_compiler->currentChunk(), name);
    int idxArgCount = func->size();
    if (idxArgCount) {
      emit((uint8_t)OpCode::OP_INTRINSIC, idxName, idxArgCount);
      for (auto itr = func->params_begin(); itr != func->params_end(); ++itr) {
        GVariantVisitor visitor;
        std::list<NodeType> ln;
        accept(*itr, &visitor, ln);
        Value arg = visitor.getVariant();
        int idxParam = addConstant(_compiler->currentChunk(), arg);
        emit(idxParam);
      }
    }
    else {
      emit((uint8_t)OpCode::OP_INTRINSIC, idxName);
    }
    
  } else {
    printf("TODO: function call not implement.\n");
  }
  return VisitFlow::SkipCurrent;
}

void GByteCodeVisitor::emit(uint8_t byte) {
  _compiler->currentChunk().push(byte);
}

void GByteCodeVisitor::namedVariant(const std::string& name, bool isAssign) {
  uint8_t opGet, opSet;
  short index = resolveLocal(_compiler, name);
  if (index == -1) {
    opSet = (uint8_t)OpCode::OP_SET_GLOBAL;
    opGet = (uint8_t)OpCode::OP_GET_GLOBAL;
  } else {
    opSet = (uint8_t)OpCode::OP_SET_LOCAL;
    opGet = (uint8_t)OpCode::OP_GET_LOCAL;
  }
  if (isAssign) {
    emit(opSet, index);
  } else {
    emit(opGet, index);
  }
}