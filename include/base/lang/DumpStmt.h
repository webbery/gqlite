#pragma once
#include <string>

class GDumpStmt {
public:
  GDumpStmt(const std::string& graph);

  std::string name() const { return _graph; }
private:
  std::string _graph;
};