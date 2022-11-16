#pragma once
#include <vector>
#include <string>
#include "base/Variant.h"

#if defined(_MSC_VER)

#define DISABLE_WARNING(warningNumber)    __pragma(warning( disable : warningNumber ))
#define DISABLE_WARNING_PUSH __pragma(warning( push ))
#define DISABLE_WARNING_POP __pragma(warning( pop ))

#elif defined(__GNUC__) || defined(__clang__)

#define DO_PRAGMA(X) _Pragma(#X)
#define DISABLE_WARNING(warningName) \
    DO_PRAGMA(GCC diagnostic ignored #warningName)
#define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
#define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)

#else
#define DISABLE_WARNING_PUSH
#define DISABLE_WARNING_POP
#endif

namespace gql {
  std::string replace_all(const std::string& input, const std::string& origin, const std::string& newer);
  std::vector<std::string> split(const char* str, const char* delim);
  std::vector<std::string> split(const std::string& str, char delim);

  /**
   * @brief try generate an identify number by input string.
   * Note: Because of hash collision, so don't use in product but experiment.
   *       So the better action is to use uint64_t as id.
   */
  uint64_t hash64(const std::string& input);

  /**
   * @brief normalize input gql. remove `"` to `'`.
   */
  std::string normalize(const std::string& gql);

  struct alignas(8) edge_id {
    bool _direction : 1;
    uint8_t _from_type : 1; // 0 means integer, otherwise bytes
    uint8_t _to_type : 1;   // 0 means integer, otherwise bytes
    uint8_t _len: 8;        // _value size, max value is 2^8 = 256
    uint8_t _from_len : 4;  // length of from, `to`'s length can be calculate from total len and `from`.
    uint8_t _reserved : 1;
    char* _value;
  };

  edge_id make_edge_id(bool direction, const Variant<std::string, uint64_t>& from, const Variant<std::string, uint64_t>& to);
  void release_edge_id(const edge_id& id);

  std::string to_string(const edge_id& id);
  edge_id to_edge_id(const std::string& id);
  bool is_same_edge_id(const edge_id& left, const edge_id& right);
  bool operator == (const edge_id& left, const edge_id& right);
  bool operator < (const edge_id& left, const edge_id& right);

  std::wstring string2wstring(const std::string& str);

  void create_directories(
#if defined(__APPLE__) || defined(UNIX) || defined(__linux__)
    const std::string& dir
#elif defined(WIN32)
    const std::wstring& dir
#endif
  );
}

template<typename T, typename Container = std::vector<T>>
bool addUniqueDataAndSort(Container& pDest, T src) {
  auto ptr = std::lower_bound(pDest.begin(), pDest.end(), src);
  if (ptr == pDest.end() || *ptr != src) {
    pDest.insert(ptr, src);
    return false;
  }
  return true;  // exist
}

template<typename T, typename Container = std::vector<T>>
bool eraseSortedData(Container& vDest, T tgt) {
  auto ptr = std::lower_bound(vDest.begin(), vDest.end(), tgt);
  if (ptr != vDest.end() && *ptr == tgt) {
    vDest.erase(ptr);
    return true;
  }
  return false;
}