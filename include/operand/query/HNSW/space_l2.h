#pragma once
#include "hnswlib.h"

namespace hnswlib {

    float
    L2Sqr(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

#if defined(USE_AVX512)

    // Favor using AVX512 if available.
    float
    L2SqrSIMD16ExtAVX512(const void *pVect1v, const void *pVect2v, const void *qty_ptr);
#endif

#if defined(USE_AVX)

    // Favor using AVX if available.
    float
    L2SqrSIMD16ExtAVX(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

#endif

#if defined(USE_SSE)

    float
    L2SqrSIMD16ExtSSE(const void *pVect1v, const void *pVect2v, const void *qty_ptr);
#endif

#if defined(USE_SSE) || defined(USE_AVX) || defined(USE_AVX512)
    extern DISTFUNC<float> L2SqrSIMD16Ext;

    float
    L2SqrSIMD16ExtResiduals(const void *pVect1v, const void *pVect2v, const void *qty_ptr);
#endif


#if defined(USE_SSE)
    float
    L2SqrSIMD4Ext(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

    float
    L2SqrSIMD4ExtResiduals(const void *pVect1v, const void *pVect2v, const void *qty_ptr);
#endif

    class L2Space : public SpaceInterface<float> {

        DISTFUNC<float> fstdistfunc_;
        size_t data_size_;
        size_t dim_;
    public:
        L2Space(size_t dim) {
            fstdistfunc_ = L2Sqr;
    #if defined(USE_SSE) || defined(USE_AVX) || defined(USE_AVX512)
        #if defined(USE_AVX512)
            if (AVX512Capable())
                L2SqrSIMD16Ext = L2SqrSIMD16ExtAVX512;
            else if (AVXCapable())
                L2SqrSIMD16Ext = L2SqrSIMD16ExtAVX;
        #elif defined(USE_AVX)
            if (AVXCapable())
                L2SqrSIMD16Ext = L2SqrSIMD16ExtAVX;
        #endif

            if (dim % 16 == 0)
                fstdistfunc_ = L2SqrSIMD16Ext;
            else if (dim % 4 == 0)
                fstdistfunc_ = L2SqrSIMD4Ext;
            else if (dim > 16)
                fstdistfunc_ = L2SqrSIMD16ExtResiduals;
            else if (dim > 4)
                fstdistfunc_ = L2SqrSIMD4ExtResiduals;
    #endif
            dim_ = dim;
            data_size_ = dim * sizeof(float);
        }

        size_t get_data_size() {
            return data_size_;
        }

        DISTFUNC<float> get_dist_func() {
            return fstdistfunc_;
        }

        void *get_dist_func_param() {
            return &dim_;
        }

        ~L2Space() {}
    };

    int
    L2SqrI4x(const void *__restrict pVect1, const void *__restrict pVect2, const void *__restrict qty_ptr);
    int L2SqrI(const void* __restrict pVect1, const void* __restrict pVect2, const void* __restrict qty_ptr);
    class L2SpaceI : public SpaceInterface<int> {

        DISTFUNC<int> fstdistfunc_;
        size_t data_size_;
        size_t dim_;
    public:
        L2SpaceI(size_t dim) {
            if(dim % 4 == 0) {
                fstdistfunc_ = L2SqrI4x;
            }
            else {
                fstdistfunc_ = L2SqrI;
            }
            dim_ = dim;
            data_size_ = dim * sizeof(unsigned char);
        }

        size_t get_data_size() {
            return data_size_;
        }

        DISTFUNC<int> get_dist_func() {
            return fstdistfunc_;
        }

        void *get_dist_func_param() {
            return &dim_;
        }

        ~L2SpaceI() {}
    };


}
