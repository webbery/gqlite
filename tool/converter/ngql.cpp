#include "ngql.h"

IConverter::Result NGQLConverter::Parse(const std::string& line, std::string& out) {
  std::cmatch m;
  std::regex regDrop("drop space \\([\\w]+\\)");
  if (std::regex_match(line.c_str(), m, regDrop)) {
    Drop(m, out);
    return IConverter::Result::CS;
  }
  std::regex regCreate("");
  if (line.find(":sleep") != std::string::npos || line.find("use") != std::string::npos) {
  }
  std::regex regUpsetVertex("^(insert vertex )\\w+\\([\\w\\W]+\\) values \"\\w+\":\\([\\w\\W]+\\)");
  if (std::regex_match(line, regUpsetVertex)) {
    UpsetVertex(m, out);
    return IConverter::Result::CS;
  }
    return IConverter::Result::CS;
}

bool NGQLConverter::Drop(std::cmatch& match, std::string& out) {
  return true;
}
bool NGQLConverter::Create(const std::string& line, std::string& out) {
  return true;
}
bool NGQLConverter::Skip(const std::string& line, std::string& out) {
  return true;
}
bool NGQLConverter::UpsetVertex(std::cmatch& match, std::string& out) {
  return true;
}
// bool NGQLConverter::UpsetEdge(std::cmatch& match, std::string& out) {}