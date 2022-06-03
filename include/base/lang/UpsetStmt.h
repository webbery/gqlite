#pragma once
#include <string>

class GUpsetStmt {
public:
  GUpsetStmt(const std::string& graphName);

  std::string name() const { return _name; }
private:
  std::string _name;
};