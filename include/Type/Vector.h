#pragma once
#include <cstdlib>
#include <stdint.h>
#include <vector>
#include <cassert>
#include <stdio.h>
#include "base/system/Platform.h"

namespace gql {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
  double sse2_distance2(const std::vector<double>& v1, const std::vector<double>& v2);
  double avx_distance2(const std::vector<double>& v1, const std::vector<double>& v2);
#elif defined(__ARM_ARCH__)
#endif
  double distance2(const std::vector<double>& v1, const std::vector<double>& v2);
  double distance2(const std::vector<double>& v);
  uint8_t distance2(const std::vector<uint8_t>& v1, const std::vector<uint8_t>& v2);
  uint8_t distance2(const std::vector<uint8_t>& v);

  bool is_same(const std::vector<double>& v1, const std::vector<double>& v2);
  bool is_same(const std::vector<uint8_t>& v1, const std::vector<uint8_t>& v2);

  template <typename T> struct VectorTraits;
  template <> struct VectorTraits<double> { using type = double; };
  template <> struct VectorTraits<float> { using type = double; };
  template <> struct VectorTraits<int> { using type = int; };
  template <> struct VectorTraits<uint64_t> { using type = double; };
  template <> struct VectorTraits<uint32_t> { using type = double; };

  template <typename Tp>
  class GVector {
  public:
    bool operator == (const GVector& other) const {
      return is_same(_vec, other._vec);
    }
    bool operator != (const GVector& other)const {
      return !is_same(_vec, other._vec);
    }
    bool operator < (const GVector& other)const {
      return distance2(_vec) < distance2(other._vec);
    }
    bool operator > (const GVector& other)const {
      return distance2(_vec) > distance2(other._vec);
    }
    bool operator >= (const GVector& other)const {
      return distance2(_vec) >= distance2(other._vec);
    }
    bool operator <= (const GVector& other)const {
      return distance2(_vec) <= distance2(other._vec);
    }

    std::vector<Tp> value()const { return _vec; }

    void push_back(const Tp& v) { _vec.push_back(v); }

  private:
    std::vector<Tp> _vec;
  };

  using vector_double = GVector<double>;
  using vector_uint8 = GVector<uint8_t>;
}
