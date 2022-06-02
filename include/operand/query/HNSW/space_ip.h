#pragma once
#include "hnswlib.h"

namespace hnswlib {

    float
    InnerProduct(const void *pVect1, const void *pVect2, const void *qty_ptr);

    float
    InnerProductDistance(const void *pVect1, const void *pVect2, const void *qty_ptr);

#if defined(USE_AVX)

// Favor using AVX if available.
    float
    InnerProductSIMD4ExtAVX(const void *pVect1v, const void *pVect2v, const void *qty_ptr);
    
    float
    InnerProductDistanceSIMD4ExtAVX(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

#endif

#if defined(USE_SSE)

    float
    InnerProductSIMD4ExtSSE(const void *pVect1v, const void *pVect2v, const void *qty_ptr);
    float
    InnerProductDistanceSIMD4ExtSSE(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

#endif


#if defined(USE_AVX512)

    float
    InnerProductSIMD16ExtAVX512(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

    float
    InnerProductDistanceSIMD16ExtAVX512(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

#endif

#if defined(USE_AVX)

    float
    InnerProductSIMD16ExtAVX(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

    float
    InnerProductDistanceSIMD16ExtAVX(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

#endif

#if defined(USE_SSE)

    float
    InnerProductSIMD16ExtSSE(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

    float
    InnerProductDistanceSIMD16ExtSSE(const void *pVect1v, const void *pVect2v, const void *qty_ptr);

#endif

#if defined(USE_SSE) || defined(USE_AVX) || defined(USE_AVX512)
    extern DISTFUNC<float> InnerProductSIMD16Ext;
    extern DISTFUNC<float> InnerProductSIMD4Ext;
    extern DISTFUNC<float> InnerProductDistanceSIMD16Ext;
    extern DISTFUNC<float> InnerProductDistanceSIMD4Ext;

    float
    InnerProductDistanceSIMD16ExtResiduals(const void *pVect1v, const void *pVect2v, const void *qty_ptr);
    float
    InnerProductDistanceSIMD4ExtResiduals(const void *pVect1v, const void *pVect2v, const void *qty_ptr);
#endif

    class InnerProductSpace : public SpaceInterface<float> {

        DISTFUNC<float> fstdistfunc_;
        size_t data_size_;
        size_t dim_;
    public:
        InnerProductSpace(size_t dim) {
            fstdistfunc_ = InnerProductDistance;
    #if defined(USE_AVX) || defined(USE_SSE) || defined(USE_AVX512)
        #if defined(USE_AVX512)
            if (AVX512Capable()) {
                InnerProductSIMD16Ext = InnerProductSIMD16ExtAVX512;
                InnerProductDistanceSIMD16Ext = InnerProductDistanceSIMD16ExtAVX512;
            } else if (AVXCapable()) {
                InnerProductSIMD16Ext = InnerProductSIMD16ExtAVX;
                InnerProductDistanceSIMD16Ext = InnerProductDistanceSIMD16ExtAVX;
            }
        #elif defined(USE_AVX)
            if (AVXCapable()) {
                InnerProductSIMD16Ext = InnerProductSIMD16ExtAVX;
                InnerProductDistanceSIMD16Ext = InnerProductDistanceSIMD16ExtAVX;
            }
        #endif
        #if defined(USE_AVX)
            if (AVXCapable()) {
                InnerProductSIMD4Ext = InnerProductSIMD4ExtAVX;
                InnerProductDistanceSIMD4Ext = InnerProductDistanceSIMD4ExtAVX;
            }
        #endif

            if (dim % 16 == 0)
                fstdistfunc_ = InnerProductDistanceSIMD16Ext;
            else if (dim % 4 == 0)
                fstdistfunc_ = InnerProductDistanceSIMD4Ext;
            else if (dim > 16)
                fstdistfunc_ = InnerProductDistanceSIMD16ExtResiduals;
            else if (dim > 4)
                fstdistfunc_ = InnerProductDistanceSIMD4ExtResiduals;
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

    ~InnerProductSpace() {}
    };

}
