#pragma once
#include <string>
#include "base/gvm/Chunk.h"

class GGQLExpression {
public:
  enum class CMDType {
    SHOW_GRAPH,
    SHOW_GRAPH_DETAIL,
    MAX
  };
  GGQLExpression(CMDType type = CMDType::MAX, const std::string& params = "");

  bool isCommand() const { return _cmdType != CMDType::MAX; }
  CMDType type() const { return _cmdType; }
  std::string params() const { return _params; }
private:
  CMDType _cmdType;
  std::string _params;
};

struct GListNode;
class GBinaryExpression {
public:
  enum class Operator {
    Add = (uint8_t)OpCode::OP_ADD,
    Subtract = (uint8_t)OpCode::OP_SUBTRACT,
    Multiply = (uint8_t)OpCode::OP_MULTIPLY,
    Divide = (uint8_t)OpCode::OP_DIVIDE,
    Assign,
    Equal,
    GreatThan,
    GreatEqualThan,
    LessThan,
    LessEqualThan,
  };

  GBinaryExpression(Operator op, GListNode* left, GListNode* right);
  ~GBinaryExpression();

  uint8_t getOperator() { return (uint8_t)_operator; }
  GListNode* left() { return _left; }
  GListNode* right() { return _right; }
private:
  Operator _operator;
  GListNode* _left;
  GListNode* _right;
};