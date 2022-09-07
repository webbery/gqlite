#include "Type/Vector.h"

namespace gql {

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
  double avx_distance2(const std::vector<double>& v1, const std::vector<double>& v2)
  {
    __m256d d1, d2;
    __m256d dis = _mm256_set1_pd(0.);
    __m256d* src1 = (__m256d*) & v1[0];
    __m256d* src2 = (__m256d*) & v2[0];
    double* result = (double*)malloc(v1.size() * sizeof(double));
    int loop = v1.size() / 4;
    for (int cnt = 0; cnt < loop; ++cnt) {
      d1 = _mm256_sub_pd(*src1, *src2);
      d2 = _mm256_mul_pd(d1, d1);
      dis = _mm256_add_pd(dis, d2);
    }
    _mm256_store_pd(result, dis);
    double distance = 0;
    for (int i = 0; i < 2; ++i) {
      distance += *(result + i);
    }
    free(result);
    return distance;
  }

  bool sse2_is_same(const std::vector<double>& v1, const std::vector<double>& v2)
  {
    if (v1.size() != v2.size()) return false;
    __m128d result = _mm_set_pd1(0.);

  }

  bool avx_is_same(const std::vector<double>& v1, const std::vector<double>& v2) {
    if (v1.size() != v2.size()) return false;
    __m256d result = _mm256_set1_pd(0.);
    __m256d* src1 = (__m256d*) & v1[0];
    __m256d* src2 = (__m256d*) & v2[0];
    int loop = v1.size() / 4;
    __m256d d1, d2;
    for (int cnt = 0; cnt < loop; ++cnt) {
      d1 = _mm256_sub_pd(*src1, *src2);
      //_mm256_s
    }
    return true;
  }
  double sse2_distance2(const std::vector<double>& v1, const std::vector<double>& v2)
  {
    __m128d d1, d2;
    __m128d dis = _mm_set_pd1(0.);
    __m128d* src1 = (__m128d*) & v1[0];
    __m128d* src2 = (__m128d*) & v2[0];
    double* result = (double*)malloc(v1.size() * sizeof(double));
    int loop = v1.size() / 2;
    for (int cnt = 0; cnt < loop; ++cnt) {
      d1 = _mm_sub_pd(*src1, *src2);
      d2 = _mm_mul_pd(d1, d1);
      dis = _mm_add_pd(dis, d2);
    }
    _mm_store_pd(result, dis);
    double distance = 0;
    for (int i = 0; i < 2; ++i) {
      distance += *(result + i);
    }
    return distance;
  }
#else
  //double neon_distance2(const std::vector<double>& v1, const std::vector<double>& v2) {
  //  int loop = v1.size() / 2;
  //  for (int cnt = 0; cnt < loop; ++cnt) {
  //    auto tv1 = vld1q_f64(&v1[2 * cnt]);
  //    auto tv2 = vld1q_f64(&v2[2 * cnt]);
  //    auto d1 = vsubq_f64(tv1, tv2);
  //    auto d = vmulq_f64(d1, d1);
  //  }
  //  return 0;
  //}
#endif

  bool is_same(const std::vector<int>& v1, const std::vector<int>& v2)
  {
    int value = 0;

    if (value == 0) return true;
    return false;
  }

  bool _is_same(const std::vector<double>& v1, const std::vector<double>& v2) {
    if (v1.size() != v2.size()) return false;
    for (size_t index = 0; index < v1.size(); ++index)
    {
      if (v1[index] != v2[index]) return false;
    }
    return true;
  }

  bool is_same(const std::vector<double>& v1, const std::vector<double>& v2)
  {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
    bool sse2 = false, avx = false, avx2 = false;
    isSSESupport(sse2, avx, avx2);
    if (avx2 || avx) {}
    else if (sse2) {
      return sse2_is_same(v1, v2);
    }
    else {
      return _is_same(v1, v2);
    }
#else
    return _is_same(v1, v2);
#endif
  }

  double distance2(const std::vector<double>& v1, const std::vector<double>& v2)
  {
    assert(v1.size() == v2.size());
    const double* p1 = &v1[0];
    const double* p2 = &v2[0];
    double v = 0;
    for (size_t i = 0; i < v1.size(); ++i) {
      v += ((*(p1 + i)) - *(p2 + i)) * (*(p1 + i) - *(p2 + i));
    }
    return v;
  }

  double distance2(const std::vector<double>& v)
  {
    double result = 0;
    const double* p = &v[0];
    for (size_t i = 0; i < v.size(); ++i) {
      result += ((*(p + i)) - *(p + i)) * (*(p + i) - *(p + i));
    }
    return result;
  }

}