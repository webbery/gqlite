#pragma once
#include "Converter.h"
#include <regex>
#include <list>

class NGQLConverter: public IConverter {
public:
  virtual Result Parse(const std::string& line, std::string& out);

private:
  bool Drop(std::cmatch& match, std::string& out);
  bool Create(std::string& out);
  bool Skip(const std::string& line, std::string& out);
  bool UpsetVertex(std::cmatch& match, std::string& out);
  bool UpsetEdge(std::cmatch& match, std::string& out);

private:
  struct Group {
    std::string _name;
    std::vector<std::string> _indexes;
    std::vector<std::string> _props;
  };

  std::string _creation;
  std::list<Group> _groups;
};