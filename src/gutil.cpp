#include "gutil.h"
#include <string.h>
#include <regex>
#include <chrono>
#include <thread>
#if defined(__APPLE__) || defined(__gnu_linux__) || defined(__linux__) 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define OS_DELIMITER "/"
#elif defined(WIN32)
#include <windows.h>
#include <direct.h>
#include <io.h>
#define OS_DELIMITER L"\\"
#define UNIVERSAL_DELIMITER L"/"
#endif
#include <codecvt>
#include <locale>


namespace gql {

  namespace {
    /**
     * upgrade of snowflake.
     * without negative bit.
     * 40-timestamp 16-input number, 8-serial number
    */
    uint64_t snowflake2(uint16_t inputID) {
      static constexpr long sequenceBit = 8;
      static constexpr long inputBit = 16;
      static constexpr long timestampShift = sequenceBit + inputBit;
      static constexpr long TWEPOCH = 1420041600000;
      thread_local long sequence = 0;
      thread_local long sequenceMask = -1L ^ (-1L << sequenceBit);
      thread_local auto lastTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      auto curTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      if (curTimeStamp == lastTimeStamp) {
        sequence = (sequenceMask + 1) & sequenceMask;
      } else {
        lastTimeStamp = curTimeStamp;
      }
      return ((curTimeStamp - TWEPOCH) << timestampShift)
        | (inputID << sequenceBit)
        | sequence;
    }

    
  void MkDir(
#if defined(__APPLE__) || defined(__gnu_linux__) || defined(__linux__) 
    const std::string& dir
#else
    const std::wstring& dir
#endif
  ) {
#if defined(__APPLE__) || defined(__gnu_linux__) || defined(__linux__) 
    mkdir(dir.c_str()
      , 0777
#else
    _wmkdir(dir.c_str()
#endif
    );
  }
  bool isDirectoryExist(
#if defined(__APPLE__) || defined(UNIX) || defined(__linux__)
    const std::string& dir
#elif defined(WIN32)
    const std::wstring& dir
#endif
  ) {
#if defined(__APPLE__) || defined(UNIX) || defined(__linux__)
    if (access(dir.c_str(), 0) != -1) return true;
#elif defined(WIN32)
    if (_waccess(dir.c_str(), 0) == 0) return true;
#endif
    return false;
  }
  
  }

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
    uint16_t id = 0;
    int times = str.size() / length;
    for (int i = 0; i < times; ++i) {
      int start = length * i;
      id += (
        uint64_t(str[start + 1]) << 8 |
        uint64_t(str[start]));
    }
    return snowflake2(id);
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

  void create_directories(
#if defined(__APPLE__) || defined(UNIX) || defined(__linux__)
    const std::string& dir
#elif defined(WIN32)
    const std::wstring& dir
#endif
  ) {
    if (isDirectoryExist(dir)) return;
    size_t pos = dir.rfind(UNIVERSAL_DELIMITER);
    if (pos == std::string::npos) {
      pos = dir.rfind(OS_DELIMITER);
      if (pos == std::string::npos) {
        MkDir(dir);
        return;
      }
    }
#if defined(__APPLE__) || defined(UNIX) || defined(__linux__)
    std::string parentDir;
#elif defined(WIN32)
    std::wstring parentDir;
#endif
    parentDir = dir.substr(0, pos);
    create_directories(parentDir);
    MkDir(dir);
  }

  std::wstring string2wstring(const std::string& str)
  {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(str);
  }
}