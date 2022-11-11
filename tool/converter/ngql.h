#pragma once
#include "Converter.h"
#include <regex>

class NGQLConverter: public IConverter {
public:
  virtual Result Parse(const std::string& line, std::string& out);

private:
  bool Drop(std::cmatch& match, std::string& out);
  bool Create(const std::string& line, std::string& out);
  bool Skip(const std::string& line, std::string& out);
  bool UpsetVertex(std::cmatch& match, std::string& out);
  bool UpsetEdge(std::cmatch& match, std::string& out);
};