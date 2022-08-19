#pragma once
#include <cstdlib>
#include <stdint.h>
#include <vector>
#include <cassert>
#include <stdio.h>

#ifdef __GNUC__
#include <x86intrin.h>
  #ifdef __SSE__
  #endif
#elif __clang__
#elif _MSC_VER
#include <intrin.h>
#endif

namespace gql {
  template <typename Tp>
  class GVector {
  public:
  private:
    std::vector<Tp> _vec;
  };

  template<typename Tp>
  Tp distance1(Tp* v1, Tp* v2) {
    
  }
#ifdef __SSE__
  inline double sse_distance2(const std::vector<double>& v1, const std::vector<double>& v2) {
    __m128d d1, d2, d3, d4;
    __m128d dis = _mm_set_pd1(0.);
    __m128d* src1 = (__m128d*)&v1[0];
    __m128d* src2 = (__m128d*)&v2[0];
    double* result = (double*)malloc(v1.size()*sizeof(double));
    int loop = v1.size() / 2;
    for (int cnt = 0; cnt < loop; ++cnt) {
      d1 = _mm_sub_pd(*src1, *src2);
      d2 = _mm_mul_pd(d1, d1);
      dis = _mm_add_pd(dis, d2);
    }
    _mm_store_pd(result, dis);
    double distance = 0;
    for (int i= 0; i< 2; ++i) {
      distance += *(result +i);
    }
    return distance;
  }
#endif

  inline double distance2(const std::vector<double>& v1, const std::vector<double>& v2) {
    assert(v1.size() == v2.size());
    const double* p1 = &v1[0];
    const double* p2 = &v2[0];
    double v = 0;
    for (size_t i = 0; i < v1.size(); ++i) {
      v += ((*(p1+i)) - *(p2+i)) * (*(p1+i) - *(p2+i));
      // printf("%f. %f, %f, %f\n", v, ((*(p1+i)) - (*p2+i)), *(p1+i), *(p2+i));
    }
    return v;
  }

  using vector_double = GVector<double>;
  using vector_uint8 = GVector<uint8_t>;
}
