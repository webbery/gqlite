#include "gutil.h"
#include <string.h>

namespace gql {

  std::vector<std::string> split(const char* str, const char* delim) {
    std::vector<std::string> result;
    char* token = strtok((char*)str, delim);
    while (token) {
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

  std::string normalize(const std::string& gql)
  {
    std::string result(gql);
    enum class ChangeStat {
      Start,
      Splash,
      End
    };
    ChangeStat previous = ChangeStat::End;
    ChangeStat cur = ChangeStat::End;
    for (size_t pos = 0, len = result.size(); pos != len; ++pos) {
      if (result[pos] == '"') {
        switch (previous) {
        case ChangeStat::Splash:
          previous = cur;
          continue;
        case ChangeStat::Start:
          cur = ChangeStat::End;
          previous = ChangeStat::End;
          break;
        case ChangeStat::End:
          cur = ChangeStat::Start;
          previous = ChangeStat::Start;
          break;
        default:break;
        }
        result[pos] = '\'';
      }
      else if (result[pos] == '\\') {
        previous = ChangeStat::Splash;
        continue;
      }
      previous = cur;
    }
    return replace_all(result, "\\u0000", "");
  }

  std::string replace_all(const std::string& input, const std::string& origin, const std::string& newer)
  {
    std::string data(input);
    size_t pos = 0;
    while ((pos = data.find(origin, pos)) != std::string::npos) {
      data = data.replace(pos, origin.size(), newer);
      pos += newer.size();
    }
    return data;
  }
}