#include "gutil.h"
#include <string.h>

std::vector<std::string> split(const char* str, const char* delim) {
  std::vector<std::string> result;
  char* token = strtok((char*)str, delim);
  while(token) {
    result.push_back(token);
    token = strtok(NULL, delim);
  }
  return result;
}