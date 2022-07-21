#pragma once
#include <string>

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