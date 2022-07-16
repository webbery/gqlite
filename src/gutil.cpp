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

uint32_t unicode32(const std::string& input)
{
  std::string str(input);
  char rest = str.size() % 4;
  if (rest != 0 || str.size() == 0) {
    rest = 4 - rest;
    for (char i = 0; i < rest; ++i) {
      str += '0';
    }
  }
  uint32_t id = 0;
  int times = str.size() / 4;
  for (int i = 0; i < times; ++i) {
    int start = 4 * i;
    id += (
      uint32_t(str[start + 3]) << 24 |
      uint32_t(str[start + 2]) << 16 |
      uint32_t(str[start + 1]) << 8 |
      uint32_t(str[start]));
  }
  return id;
}
