#include "Type/QueryCondition.h"
#include "Error.h"
#include "base/ast.h"

namespace {
  NodeType getJsonType(const nlohmann::json& js) {
    switch (js.type())
    {
    case nlohmann::json::value_t::object:
      return NodeType::ObjectExpression;
    case nlohmann::json::value_t::number_integer:
      return NodeType::Integer;
    case nlohmann::json::value_t::string:
      return NodeType::String;
    default:
      return NodeType::Removable;
    }
  }

  std::any getJsonValue(const nlohmann::json& js) {
    switch (js.type()) {
    case nlohmann::json::value_t::number_integer:
      return js.get<int32_t>();
    case nlohmann::json::value_t::string:
      return js.get<std::string>();
    default:
      return nullptr;
    }
  }
}

class ConditionVisitor {
public:
  ConditionVisitor() {
    _value._isAnd = true;
  }

  void visit(Acceptor<NodeType::String>& acceptor) {
    printf("String: %s\n", acceptor.value().c_str());
  }

  void visit(Acceptor<NodeType::Number>& acceptor) {
    printf("Number: %f\n", acceptor.value());
  }

  void visit(Acceptor<NodeType::Integer>& acceptor) {
    printf("Integer: %d\n", acceptor.value());
  }

  void visit(Acceptor<NodeType::Property>& acceptor) {
    auto& value = acceptor.value();
    std::shared_ptr<GPredition> curPred(new GPredition());
    std::shared_ptr<GPredition> root = curPred;
    printf("%s\n", acceptor.value().dump().c_str());
    if (acceptor.value().size() == 0) return;
    for (auto itr = value.begin(), end = value.end(); itr != end; ++itr) {
      GPredition* pred = new GPredition();
      pred->_indx = itr.key();
      pred->_fn = itr.key();
      pred->_type = getJsonType(itr.value());
      pred->_value = getJsonValue(itr.value());
      curPred->_next = std::shared_ptr<GPredition>(pred);
      curPred = curPred->_next;
    }
    _value._preds = root->_next;
    printf("Property: %s\n", acceptor.value().dump().c_str());
  }

  void visit(Acceptor<NodeType::ArrayExpression>& acceptor) {
    printf("ArrayExpression: %s\n", acceptor.value().dump().c_str());
  }

  void visit(Acceptor<NodeType::ObjectExpression>& acceptor) {
    printf("ObjectExpression: %s\n", acceptor.value().dump().c_str());
  }

  GConditions value() { return _value; }
private:
  GConditions _value;
};

int GQueryConditionClause::Parse(struct gast* ast)
{
  bool isBin = false;
  //ArrayVisitor cv;
  ConditionVisitor cv;
  GET_VALUE(ast, isBin, cv);
  _preds = cv.value();
  //printf("result: %s\n", cv.value().dump().c_str());
  return ECode_Success;
}
