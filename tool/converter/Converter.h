#pragma once
#include <string>
#include <vector>
#include <string.h>

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

inline std::vector<std::string> split(const char* str, const char* delim) {
  std::vector<std::string> result;
  char* token = strtok((char*)str, delim);
  while (token) {
    result.push_back(token);
    token = strtok(NULL, delim);
  }
  return result;
}