#pragma once
#include <string>

class IConverter {
public:
  enum Result{
    CS,   // Convert Success,
    RN,   // Read Next, it means that one line is not suffient
    CF,   // Convert Fail.
  };
  virtual Result Parse(const std::string& line, std::string& out) = 0;
  virtual ~IConverter() {}
};