#pragma once
#include <cstdlib>
#include <stdint.h>
#include <vector>
#include <cassert>
#include <stdio.h>
#include "base/system/Platform.h"

#ifdef _WIN32
#include <intrin.h>
#else
  #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
    #include <x86intrin.h>
  #elif defined(__ARM_ARCH__)
#include <arm_neon.h>
  #endif
#endif

namespace gql {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
  double sse2_distance2(const std::vector<double>& v1, const std::vector<double>& v2);
  double avx_distance2(const std::vector<double>& v1, const std::vector<double>& v2);
#elif defined(__ARM_ARCH__)
#endif
  double distance2(const std::vector<double>& v1, const std::vector<double>& v2);
  double distance2(const std::vector<double>& v);

  bool is_same(const std::vector<double>& v1, const std::vector<double>& v2);
  bool is_same(const std::vector<int>& v1, const std::vector<int>& v2);

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

  private:
    std::vector<Tp> _vec;
  };

  using vector_double = GVector<double>;
  using vector_uint8 = GVector<uint8_t>;
}
