#include "Type/QueryCondition.h"
#include "Error.h"
#include "base/ast.h"

class ConditionVisitor {
public:
  ConditionVisitor() {
    _preds._isAnd = true;
  }

  void visit(Acceptor<NodeType::String>& acceptor) {
    printf("String: %s\n", acceptor.value().c_str());
  }

  void visit(Acceptor<NodeType::Number>& acceptor) {
    printf("Number: %d\n", acceptor.value());
  }

  void visit(Acceptor<NodeType::Property>& acceptor) {
    printf("Property: %s\n", acceptor.value().dump().c_str());
  }

  void visit(Acceptor<NodeType::ArrayExpression>& acceptor) {
    printf("ArrayExpression: %s\n", acceptor.value().dump().c_str());
  }

  GConditions value() { return _preds; }
private:
  GConditions _preds;
};

int GQueryConditionClause::Parse(struct gast* ast)
{
  bool isBin = false;
  ArrayVisitor cv;
  //ConditionVisitor cv;
  GET_VALUE(ast, isBin, cv);
  //_preds = cv.value();
  printf("result: %s\n", cv.value().dump().c_str());
  return ECode_Success;
}
