#include "gutil.h"
#include <string.h>
#include <regex>

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

  std::vector<std::string> split(const std::string& str, char delim)
  {
    std::vector<std::string> result;
    auto first = str.begin();
    for (auto itr = str.begin(); itr != str.end(); ++itr) {
      if (*itr == delim) {
        result.push_back({ first, itr });
        first = itr;
      }
    }
    if (first != str.end()) {
      result.push_back({ first, str.end() });
    }
    return result;
  }

  uint64_t hash64(const std::string& input)
  {
    constexpr short length = 8;
    std::string str(input);
    char rest = str.size() % length;
    if (rest != 0 || str.size() == 0) {
      rest = length - rest;
      for (char i = 0; i < rest; ++i) {
        str += '0';
      }
    }
    uint64_t id = 0;
    int times = str.size() / length;
    for (int i = 0; i < times; ++i) {
      int start = length * i;
      id += (
        uint64_t(str[start + 7]) << 56 |
        uint64_t(str[start + 6]) << 48 |
        uint64_t(str[start + 5]) << 40 |
        uint64_t(str[start + 4]) << 32 |
        uint64_t(str[start + 3]) << 24 |
        uint64_t(str[start + 2]) << 16 |
        uint64_t(str[start + 1]) << 8 |
        uint64_t(str[start]));
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

    result = replace_all(result, "\\u0000", "");
    // convert datetime, array
    std::regex patternDatetime("\\{'_obj_t':4,'value':(\\d+)\\}");
    result = std::regex_replace(result, patternDatetime, "0d$1");
    std::regex patternArray("\\{'_obj_t':5,'value':(\\[[\\w\\W]+\\])\\}");
    return std::regex_replace(result, patternArray, "$1");
  }

  edge_id make_edge_id(bool direction, const Variant<std::string, uint64_t>& from, const Variant<std::string, uint64_t>& to)
  {
    edge_id id;
    id._reserved = 0;
    id._direction = direction;
    id._from_type = !from.index();
    id._to_type = !to.index();
    id._from_len = (from.index() == 0 ? from.Get<std::string>().size() : sizeof(uint64_t));
    short to_len = (to.index() == 0 ? to.Get<std::string>().size() : sizeof(uint64_t));
    id._len = to_len + id._from_len;
    id._value = (char*)malloc(id._len);
    if (from.index() == 0) {
      memcpy(id._value, from.Get<std::string>().data(), id._from_len);
    }
    else {
      memcpy(id._value, &from.Get<uint64_t>(), id._from_len);
    }
    if (to.index() == 0) {
      memcpy(id._value + id._from_len, to.Get<std::string>().data(), to_len);
    }
    else {
      memcpy(id._value + id._from_len, &to.Get<uint64_t>(), to_len);
    }
    return id;
  }

  void release_edge_id(const edge_id& id)
  {
    free(id._value);
  }

  std::string to_string(const edge_id& id)
  {
    uint8_t info[2] = { 0 };
    memcpy(info, &id, 2 * sizeof(uint8_t));
    std::string s((char*)info, 2);
    s.append(id._value, id._len);
    return s;
  }

  edge_id to_edge_id(const std::string& id)
  {
    edge_id eid = { 0 };
    memcpy(&eid, id.data(), 2);
    eid._value = (char*)malloc(eid._len);
    memcpy(eid._value, id.data() + 2, id.size() - 2);
    return eid;
  }

  bool is_same_edge_id(const edge_id& left, const edge_id& right)
  {
    if (left._direction != right._direction || left._len != right._len || left._from_len != right._from_len
      || left._from_type != right._from_type || left._to_type != right._to_type) return false;
    return memcmp(left._value, right._value, right._len) == 0;
  }

  bool operator<(const edge_id& left, const edge_id& right)
  {
    if (left._len < right._len || left._from_len < right._from_len) return true;
    return false;
  }

  bool operator==(const edge_id& left, const edge_id& right)
  {
    return is_same_edge_id(left, right);
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