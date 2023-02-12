#pragma once

#include "base/lang/ObjectFunction.h"
#include "base/lang/visitor/IVisitor.h"
#include "base/gvm/Compiler.h"
#include "base/lang/AST.h"
#include "base/type.h"

class GVM;
class GByteCodeVisitor: public GVisitor {
public:
  Compiler* _compiler;

  GByteCodeVisitor(GVM* gvm, Compiler* compiler = nullptr);

  VisitFlow apply(GReturnStmt* stmt, std::list<NodeType>& path);

  VisitFlow apply(GBlockStmt* stmt, std::list<NodeType>& path);

  VisitFlow apply(GBinaryExpression* stmt, std::list<NodeType>& path);

  VisitFlow apply(GAssignStmt* stmt, std::list<NodeType>& path);

  VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path);

  VisitFlow apply(GObjectFunction* func, std::list<NodeType>& path);

private:
    /**
    * @brief For graph script, this function emit byte code to chunk.
    *        Then gvm will envoke the byte code and other plan will retrieve the result.
    */
    void emit(uint8_t byte);
    
    template<typename... Types>
    void emit(uint8_t byte, Types... args) {
      _compiler->currentChunk().push(byte);
      emit(args...);
    }

    void namedVariant(const std::string& name, bool isAssign);
private:
  GVM* _gvm;
};