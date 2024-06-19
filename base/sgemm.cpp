// -*- mode:c++;indent-tabs-mode:nil;c-basic-offset:4;coding:utf-8 -*-
// vi: set et ft=c++ ts=4 sts=4 sw=4 fenc=utf-8 :vi
//
// Copyright 2024 Mozilla Foundation
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//
//                   _   _          ___ _      _   ___
//                  | |_(_)_ _ _  _| _ ) |    /_\ / __|
//                  |  _| | ' \ || | _ \ |__ / _ \\__ \.
//                   \__|_|_||_\_, |___/____/_/ \_\___/
//                             |__/
//
//                    BASIC LINEAR ALGEBRA SUBPROGRAMS
//
//
// This file implements multithreaded CPU matrix multiplication for the
// common contiguous use case C = Aᵀ * B. These kernels are designed to
// have excellent performance[1] for matrices that fit in the CPU cache
// without imposing any overhead such as cache filling or malloc calls.
//
// This implementation does not guarantee any upper bound with rounding
// errors, which grow along with k. Our goal's to maximally exploit the
// hardware for performance, and then use whatever resources remain for
// improving numerical accuracy.
//
// [1] J. Tunney, ‘LLaMA Now Goes Faster on CPUs’, Mar. 2024. [Online].
//     Available: https://justine.lol/matmul/. [Accessed: 29-Mar-2024].

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wignored-attributes"
#endif

#include "sgemm.h"
#include <algorithm>
#include "ggml-impl.h"
#include "ggml-quants.h"

#define ROW_ALIGN 64
#define MATRIX_ALIGN 4096
#define MAX_ALIGN 4096

#ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((__noinline__))
#endif

#if defined(__ARM_NEON) || defined(__AVX512F__)
#define VECTOR_REGISTERS 32
#else
#define VECTOR_REGISTERS 16
#endif

namespace {

inline float unhalf(ggml_fp16_t d) {
    return GGML_FP16_TO_FP32(d);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// MATRIX MEMORY INDEXING

#define NCA 1
#define NCB 2
#define NCC 4

#define INDEX(A, lda, j, i) index<CONFIG & NC##A>(A, lda, j, i)

template<int NC, typename T>
inline T &index(T *A, int lda, int j, int i) {
    if (NC)
        return ((T **)A)[j][i];
    else
        return A[lda * j + i];
}

template<int NC, typename T>
inline const T &index(const T *A, int lda, int j, int i) {
    if (NC)
        return ((const T *const *)A)[j][i];
    else
        return A[lda * j + i];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// GGML TYPE TRAITS

template<typename T> struct ggml_type_trait;
template<> struct ggml_type_trait<float>        { static constexpr ggml_type id = GGML_TYPE_F32;  };
template<> struct ggml_type_trait<ggml_fp16_t>  { static constexpr ggml_type id = GGML_TYPE_F16;  };
template<> struct ggml_type_trait<block_q8_0>   { static constexpr ggml_type id = GGML_TYPE_Q8_0; };

////////////////////////////////////////////////////////////////////////////////////////////////////
// VECTORIZED ARITHMETIC OPERATIONS

#if defined(__SSE__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
inline __m128 add(__m128 x, __m128 y) { return _mm_add_ps(x, y); }
inline __m128 sub(__m128 x, __m128 y) { return _mm_sub_ps(x, y); }
inline __m128 mul(__m128 x, __m128 y) { return _mm_mul_ps(x, y); }
#endif  // __SSE__

#if defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
inline __m256 add(__m256 x, __m256 y) { return _mm256_add_ps(x, y); }
inline __m256 sub(__m256 x, __m256 y) { return _mm256_sub_ps(x, y); }
inline __m256 mul(__m256 x, __m256 y) { return _mm256_mul_ps(x, y); }
#endif // __AVX__

#if defined(__AVX512F__)
inline __m512 add(__m512 x, __m512 y) { return _mm512_add_ps(x, y); }
inline __m512 sub(__m512 x, __m512 y) { return _mm512_sub_ps(x, y); }
inline __m512 mul(__m512 x, __m512 y) { return _mm512_mul_ps(x, y); }
#endif // __AVX512F__

#if defined(__ARM_NEON)
inline float32x4_t add(float32x4_t x, float32x4_t y) { return vaddq_f32(x, y); }
inline float32x4_t sub(float32x4_t x, float32x4_t y) { return vsubq_f32(x, y); }
inline float32x4_t mul(float32x4_t x, float32x4_t y) { return vmulq_f32(x, y); }
#endif // __ARM_NEON

#if defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC)
inline float16x8_t add(float16x8_t x, float16x8_t y) { return vaddq_f16(x, y); }
inline float16x8_t sub(float16x8_t x, float16x8_t y) { return vsubq_f16(x, y); }
inline float16x8_t mul(float16x8_t x, float16x8_t y) { return vmulq_f16(x, y); }
#endif // __ARM_FEATURE_FP16_VECTOR_ARITHMETIC

////////////////////////////////////////////////////////////////////////////////////////////////////
// VECTORIZED FUSED MULTIPLY ADD

/**
 * Computes a * b + c.
 */
template <typename T, typename U>
inline U madd(T a, T b, U c) {
    return add(mul(a, b), c);
}

#if defined(__FMA__)
#if defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
template <>
inline __m256 madd(__m256 a, __m256 b, __m256 c) {
    return _mm256_fmadd_ps(a, b, c);
}
#endif
#if defined(__AVX512F__)
template <>
inline __m512 madd(__m512 a, __m512 b, __m512 c) {
    return _mm512_fmadd_ps(a, b, c);
}
#endif
#endif

#if defined(__ARM_FEATURE_FMA)
template <>
inline float32x4_t madd(float32x4_t a, float32x4_t b, float32x4_t c) {
    return vfmaq_f32(c, b, a);
}
#if defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC) && !defined(_MSC_VER)
template <>
inline float16x8_t madd(float16x8_t a, float16x8_t b, float16x8_t c) {
    return vfmaq_f16(c, b, a);
}
#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// VECTORIZED HORIZONTAL SUM

#if defined(__ARM_NEON)
inline float hsum(float32x4_t x) {
    return vaddvq_f32(x);
}
#endif // __ARM_NEON

#if defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC) && !defined(_MSC_VER)
inline float hsum(float16x8_t x) {
    return vaddvq_f32(vaddq_f32(vcvt_f32_f16(vget_low_f16(x)),
                                vcvt_f32_f16(vget_high_f16(x))));
}
#endif // __ARM_FEATURE_FP16_VECTOR_ARITHMETIC

#if defined(__SSE__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
inline float hsum(__m128 x) {
#if defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
    x = _mm_add_ps(x, _mm_movehl_ps(x, x));
    x = _mm_add_ss(x, _mm_movehdup_ps(x));
#else
    __m128 t;
    t = _mm_shuffle_ps(x, x, _MM_SHUFFLE(2, 3, 0, 1));
    x = _mm_add_ps(x, t);
    t = _mm_movehl_ps(t, x);
    x = _mm_add_ss(x, t);
#endif
    return _mm_cvtss_f32(x);
}
#endif

#if defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
inline float hsum(__m256 x) {
    return hsum(_mm_add_ps(_mm256_extractf128_ps(x, 1),
                           _mm256_castps256_ps128(x)));
}
#endif // __AVX__

#if defined(__AVX512F__)
inline float hsum(__m512 x) {
    return _mm512_reduce_add_ps(x);
}
#endif // __AVX512F__

////////////////////////////////////////////////////////////////////////////////////////////////////
// VECTORIZED MEMORY LOADING

template <typename T, typename U> T load(const U *);

#if defined(__ARM_NEON)
template <> inline float32x4_t load(const float *p) {
    return vld1q_f32(p);
}
#if !defined(_MSC_VER)
template <> inline float16x8_t load(const ggml_fp16_t *p) {
    return vld1q_f16((const float16_t *)p);
}
template <> inline float32x4_t load(const ggml_fp16_t *p) {
    return vcvt_f32_f16(vld1_f16((const float16_t *)p));
}
#endif // _MSC_VER
#endif // __ARM_NEON

#if defined(__SSE__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
template <> inline __m128 load(const float *p) {
    return _mm_loadu_ps(p);
}
#endif  // __SSE__

#if defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
template <> inline __m256 load(const float *p) {
    return _mm256_loadu_ps(p);
}
#endif // __AVX__

#if defined(__F16C__)
template <> inline __m256 load(const ggml_fp16_t *p) {
    return _mm256_cvtph_ps(_mm_loadu_si128((const __m128i *)p));
}
#endif // __F16C__

#if defined(__AVX512F__)
template <> inline __m512 load(const float *p) {
    return _mm512_loadu_ps(p);
}
template <> inline __m512 load(const ggml_fp16_t *p) {
    return _mm512_cvtph_ps(_mm256_loadu_si256((const __m256i *)p));
}
#endif // __AVX512F__

////////////////////////////////////////////////////////////////////////////////////////////////////
// FLOATING POINT MATRIX MULTIPLICATION

template <int CONFIG, int KN, typename D, typename V, typename TA, typename TB, typename TC>
class tinyBLAS {
  public:
    tinyBLAS(int k,
             const TA *A, int lda,
             const TB *B, int ldb,
             TC *C, int ldc,
             int ith, int nth)
        : A(A), B(B), C(C), k(k), lda(lda), ldb(ldb), ldc(ldc), ith(ith), nth(nth) {
    }

    void matmul(int m, int n, int task) {
        if (task == GGML_TASK_TYPE_COMPUTE)
            mnpack(0, m, 0, n);
    }

  private:
    NOINLINE void mnpack(int m0, int m, int n0, int n) {
        int mc, nc, mp, np;
        switch ((std::min(m - m0, 5) << 4) | std::min(n - n0, 5)) {
#if VECTOR_REGISTERS == 32
        case 0x55:
            mc = 5;
            nc = 5;
            gemm<5, 5>(m0, m, n0, n);
            break;
        case 0x45:
            mc = 4;
            nc = 5;
            gemm<4, 5>(m0, m, n0, n);
            break;
        case 0x54:
            mc = 5;
            nc = 4;
            gemm<5, 4>(m0, m, n0, n);
            break;
        case 0x44:
            mc = 4;
            nc = 4;
            gemm<4, 4>(m0, m, n0, n);
            break;
        case 0x53:
            mc = 5;
            nc = 3;
            gemm<5, 3>(m0, m, n0, n);
            break;
        case 0x35:
            mc = 3;
            nc = 5;
            gemm<3, 5>(m0, m, n0, n);
            break;
        case 0x43:
            mc = 4;
            nc = 3;
            gemm<4, 3>(m0, m, n0, n);
            break;
#else
        case 0x55:
        case 0x54:
        case 0x53:
        case 0x45:
        case 0x44:
        case 0x43:
            mc = 4;
            nc = 3;
            gemm<4, 3>(m0, m, n0, n);
            break;
        case 0x35:
#endif
        case 0x34:
            mc = 3;
            nc = 4;
            gemm<3, 4>(m0, m, n0, n);
            break;
        case 0x52:
            mc = 5;
            nc = 2;
            gemm<5, 2>(m0, m, n0, n);
            break;
        case 0x33:
            mc = 3;
            nc = 3;
            gemm<3, 3>(m0, m, n0, n);
            break;
        case 0x25:
            mc = 2;
            nc = 5;
            gemm<2, 5>(m0, m, n0, n);
            break;
        case 0x42:
            mc = 4;
            nc = 2;
            gemm<4, 2>(m0, m, n0, n);
            break;
        case 0x24:
            mc = 2;
            nc = 4;
            gemm<2, 4>(m0, m, n0, n);
            break;
        case 0x32:
            mc = 3;
            nc = 2;
            gemm<3, 2>(m0, m, n0, n);
            break;
        case 0x23:
            mc = 2;
            nc = 3;
            gemm<2, 3>(m0, m, n0, n);
            break;
        case 0x51:
            mc = 5;
            nc = 1;
            gemm<5, 1>(m0, m, n0, n);
            break;
        case 0x41:
            mc = 4;
            nc = 1;
            gemm<4, 1>(m0, m, n0, n);
            break;
        case 0x22:
            mc = 2;
            nc = 2;
            gemm<2, 2>(m0, m, n0, n);
            break;
        case 0x15:
            mc = 1;
            nc = 5;
            gemm<1, 5>(m0, m, n0, n);
            break;
        case 0x14:
            mc = 1;
            nc = 4;
            gemm<1, 4>(m0, m, n0, n);
            break;
        case 0x31:
            mc = 3;
            nc = 1;
            gemm<3, 1>(m0, m, n0, n);
            break;
        case 0x13:
            mc = 1;
            nc = 3;
            gemm<1, 3>(m0, m, n0, n);
            break;
        case 0x21:
            mc = 2;
            nc = 1;
            gemm<2, 1>(m0, m, n0, n);
            break;
        case 0x12:
            mc = 1;
            nc = 2;
            gemm<1, 2>(m0, m, n0, n);
            break;
        case 0x11:
            mc = 1;
            nc = 1;
            gemm<1, 1>(m0, m, n0, n);
            break;
        default:
            return;
        }
        mp = m0 + (m - m0) / mc * mc;
        np = n0 + (n - n0) / nc * nc;
        mnpack(mp, m, n0, np);
        mnpack(m0, m, np, n);
    }

    template <int RM, int RN>
    NOINLINE void gemm(int m0, int m, int n0, int n) {
        int ytiles = RM > 1 ? (m - m0) / RM : 1;
        int xtiles = RN > 1 ? (n - n0) / RN : 1;
        int tiles = xtiles * ytiles;
        int duty = (tiles + nth - 1) / nth;
        int start = duty * ith;
        int end = start + duty;
        if (end > tiles)
            end = tiles;
        for (int job = start; job < end; ++job) {
            int ii = m0 + job / xtiles * RM;
            int jj = n0 + job % xtiles * RN;
            D Cv[RN][RM] = {};
            for (int l = 0; l < k; l += KN)
                for (int j = 0; j < RN; ++j)
                    for (int i = 0; i < RM; ++i)
                        Cv[j][i] = madd(load<V>(&INDEX(A, lda, ii + i, l)),
                                        load<V>(&INDEX(B, ldb, jj + j, l)),
                                        Cv[j][i]);
            for (int j = 0; j < RN; ++j)
                for (int i = 0; i < RM; ++i)
                    INDEX(C, ldc, jj + j, ii + i) = hsum(Cv[j][i]);
        }
    }

    const TA *const A;
    const TB *const B;
    TC *const C;
    const int k;
    const int lda;
    const int ldb;
    const int ldc;
    const int ith;
    const int nth;
};

//////////////////////////////////////////////////////////////////////////////////////////
// QUANT ZERO MATRIX MULTIPLICATION

#if defined(__ARM_FEATURE_DOTPROD)
template <int CONFIG, typename TA>
class tinyBLAS_Q0_ARM {
  public:
    tinyBLAS_Q0_ARM(int k,
                    const TA *A, int lda,
                    const block_q8_0 *B, int ldb,
                    float *C, int ldc,
                    int ith, int nth)
        : A(A), B(B), C(C), k(k), lda(lda), ldb(ldb), ldc(ldc), ith(ith), nth(nth) {
    }

    void matmul(int m, int n, int task) {
        if (task == GGML_TASK_TYPE_COMPUTE)
            mnpack(0, m, 0, n);
    }

  private:
    NOINLINE void mnpack(int m0, int m, int n0, int n) {
        int mc, nc, mp, np;
        switch ((std::min(m - m0, 3) << 4) | std::min(n - n0, 3)) {
        case 0x33:
            mc = 3;
            nc = 3;
            gemm<3, 3>(m0, m, n0, n);
            break;
        case 0x32:
            mc = 3;
            nc = 2;
            gemm<3, 2>(m0, m, n0, n);
            break;
        case 0x23:
            mc = 2;
            nc = 3;
            gemm<2, 3>(m0, m, n0, n);
            break;
        case 0x22:
            mc = 2;
            nc = 2;
            gemm<2, 2>(m0, m, n0, n);
            break;
        case 0x31:
            mc = 3;
            nc = 1;
            gemm<3, 1>(m0, m, n0, n);
            break;
        case 0x13:
            mc = 1;
            nc = 3;
            gemm<1, 3>(m0, m, n0, n);
            break;
        case 0x21:
            mc = 2;
            nc = 1;
            gemm<2, 1>(m0, m, n0, n);
            break;
        case 0x12:
            mc = 1;
            nc = 2;
            gemm<1, 2>(m0, m, n0, n);
            break;
        case 0x11:
            mc = 1;
            nc = 1;
            gemm<1, 1>(m0, m, n0, n);
            break;
        default:
            return;
        }
        mp = m0 + (m - m0) / mc * mc;
        np = n0 + (n - n0) / nc * nc;
        mnpack(mp, m, n0, np);
        mnpack(m0, m, np, n);
    }

    template <int RM, int RN>
    NOINLINE void gemm(int m0, int m, int n0, int n) {
        int ytiles = RM > 1 ? (m - m0) / RM : 1;
        int xtiles = RN > 1 ? (n - n0) / RN : 1;
        int tiles = xtiles * ytiles;
        int duty = (tiles + nth - 1) / nth;
        int start = duty * ith;
        int end = start + duty;
        if (end > tiles)
            end = tiles;
        for (int job = start; job < end; ++job) {
            int ii = m0 + job / xtiles * RM;
            int jj = n0 + job % xtiles * RN;
            float32x4_t Cv[RN][RM] = {};
            for (int l = 0; l < k; ++l)
                for (int j = 0; j < RN; ++j)
                    for (int i = 0; i < RM; ++i)
                        Cv[j][i] = vmlaq_n_f32(Cv[j][i],
                                               vcvtq_f32_s32(vdotq_s32(
                                                   vdotq_s32(vdupq_n_s32(0),
                                                             load_lo(&INDEX(A, lda, ii + i, l)),
                                                             load_lo(&INDEX(B, ldb, jj + j, l))),
                                                   load_hi(&INDEX(A, lda, ii + i, l)),
                                                   load_hi(&INDEX(B, ldb, jj + j, l)))),
                                               unhalf(INDEX(A, lda, ii + i, l).d) *
                                               unhalf(INDEX(B, ldb, jj + j, l).d));
            for (int j = 0; j < RN; ++j)
                for (int i = 0; i < RM; ++i)
                    INDEX(C, ldc, jj + j, ii + i) = hsum(Cv[j][i]);
        }
    }

    inline int8x16_t load_lo(const block_q8_0 *b) {
        return vld1q_s8(b->qs);
    }

    inline int8x16_t load_hi(const block_q8_0 *b) {
        return vld1q_s8(b->qs + 16);
    }

    inline int8x16_t load_lo(const block_q4_0 *b) {
        return vsubq_s8(vreinterpretq_s8_u8(vandq_u8(vld1q_u8(b->qs),
                                                     vdupq_n_u8(0x0f))),
                        vdupq_n_s8(0x8));
    }

    inline int8x16_t load_hi(const block_q4_0 *b) {
        return vsubq_s8(vreinterpretq_s8_u8(vshrq_n_u8(vld1q_u8(b->qs), 4)),
                        vdupq_n_s8(0x8));
    }

    const TA *const A;
    const block_q8_0 *const B;
    float *const C;
    const int k;
    const int lda;
    const int ldb;
    const int ldc;
    const int ith;
    const int nth;
};
#endif // __ARM_FEATURE_DOTPROD

#if defined(__AVX2__) || defined(__AVX512F__)
template <int CONFIG, typename TA, typename TB, typename TC>
class tinyBLAS_Q0_AVX2 {
  public:
    tinyBLAS_Q0_AVX2(int k,
                     const TA *A, int lda,
                     const TB *B, int ldb,
                     TC *C, int ldc,
                     int ith, int nth)
        : A(A), B(B), C(C), k(k), lda(lda), ldb(ldb), ldc(ldc), ith(ith), nth(nth) {
    }

    void matmul(int m, int n, int task) {
        if (task == GGML_TASK_TYPE_COMPUTE)
            mnpack(0, m, 0, n);
    }

  private:
    void mnpack(int m0, int m, int n0, int n) {
        int mc, nc, mp, np;
        switch ((std::min(m - m0, 4) << 4) | std::min(n - n0, 4)) {
#if VECTOR_REGISTERS == 32
        case 0x44:
            mc = 4;
            nc = 4;
            gemm<4, 4>(m0, m, n0, n);
            break;
        case 0x43:
            mc = 4;
            nc = 3;
            gemm<4, 3>(m0, m, n0, n);
            break;
        case 0x34:
            mc = 3;
            nc = 4;
            gemm<3, 4>(m0, m, n0, n);
            break;
        case 0x33:
            mc = 3;
            nc = 3;
            gemm<3, 3>(m0, m, n0, n);
            break;
        case 0x42:
            mc = 4;
            nc = 2;
            gemm<4, 2>(m0, m, n0, n);
            break;
        case 0x24:
            mc = 2;
            nc = 4;
            gemm<2, 4>(m0, m, n0, n);
            break;
#else
        case 0x44:
        case 0x43:
        case 0x42:
            mc = 4;
            nc = 2;
            gemm<4, 2>(m0, m, n0, n);
            break;
        case 0x34:
        case 0x24:
            mc = 2;
            nc = 4;
            gemm<2, 4>(m0, m, n0, n);
            break;
        case 0x33:
#endif
        case 0x32:
            mc = 3;
            nc = 2;
            gemm<3, 2>(m0, m, n0, n);
            break;
        case 0x23:
            mc = 2;
            nc = 3;
            gemm<2, 3>(m0, m, n0, n);
            break;
        case 0x41:
            mc = 4;
            nc = 1;
            gemm<4, 1>(m0, m, n0, n);
            break;
        case 0x22:
            mc = 2;
            nc = 2;
            gemm<2, 2>(m0, m, n0, n);
            break;
        case 0x14:
            mc = 1;
            nc = 4;
            gemm<1, 4>(m0, m, n0, n);
            break;
        case 0x31:
            mc = 3;
            nc = 1;
            gemm<3, 1>(m0, m, n0, n);
            break;
        case 0x13:
            mc = 1;
            nc = 3;
            gemm<1, 3>(m0, m, n0, n);
            break;
        case 0x21:
            mc = 2;
            nc = 1;
            gemm<2, 1>(m0, m, n0, n);
            break;
        case 0x12:
            mc = 1;
            nc = 2;
            gemm<1, 2>(m0, m, n0, n);
            break;
        case 0x11:
            mc = 1;
            nc = 1;
            gemm<1, 1>(m0, m, n0, n);
            break;
        default:
            return;
        }
        mp = m0 + (m - m0) / mc * mc;
        np = n0 + (n - n0) / nc * nc;
        mnpack(mp, m, n0, np);
        mnpack(m0, m, np, n);
    }

    template <int RM, int RN>
    NOINLINE void gemm(int m0, int m, int n0, int n) {
        int ytiles = RM > 1 ? (m - m0) / RM : 1;
        int xtiles = RN > 1 ? (n - n0) / RN : 1;
        int tiles = xtiles * ytiles;
        int duty = (tiles + nth - 1) / nth;
        int start = duty * ith;
        int end = start + duty;
        if (end > tiles)
            end = tiles;
        for (int job = start; job < end; ++job) {
            int ii = m0 + job / xtiles * RM;
            int jj = n0 + job % xtiles * RN;
            __m256 Cv[RN][RM] = {};
            for (int l = 0; l < k; ++l)
                for (int j = 0; j < RN; ++j)
                    for (int i = 0; i < RM; ++i)
                        Cv[j][i] = madd(_mm256_set1_ps(unhalf(INDEX(A, lda, ii + i, l).d) *
                                                       unhalf(INDEX(B, ldb, jj + j, l).d)),
                                        updot(_mm256_sign_epi8(load(&INDEX(A, lda, ii + i, l)),
                                                               load(&INDEX(A, lda, ii + i, l))),
                                              _mm256_sign_epi8(load(&INDEX(B, ldb, jj + j, l)),
                                                               load(&INDEX(A, lda, ii + i, l)))),
                                        Cv[j][i]);
            for (int j = 0; j < RN; ++j)
                for (int i = 0; i < RM; ++i)
                    INDEX(C, ldc, jj + j, ii + i) = hsum(Cv[j][i]);
        }
    }

    inline __m256i load(const block_q8_0 *b) {
        return _mm256_loadu_si256((const __m256i *)b->qs);
    }

    inline __m256i load(const block_q4_0 *b) {
        return _mm256_sub_epi8(denibble(b->qs), _mm256_set1_epi8(8));
    }

    inline __m256 updot(__m256i u, __m256i s) {
        __m256i res;
#if defined(__AVXVNNI__) || (defined(__AVX512VNNI__) && defined(__AVX512VL__))
        res = _mm256_dpbusd_epi32(_mm256_setzero_si256(), u, s);
#else
        res = _mm256_madd_epi16(_mm256_set1_epi16(1), _mm256_maddubs_epi16(u, s));
#endif
        return _mm256_cvtepi32_ps(res);
    }

    static inline __m256i denibble(const uint8_t *p) {
        __m128i x = _mm_loadu_si128((const __m128i *)p);
        return _mm256_and_si256(_mm256_set1_epi8(15),
                                _mm256_insertf128_si256(_mm256_castsi128_si256(x),
                                                        _mm_srli_epi16(x, 4), 1));
    }

    const TA *const A;
    const TB *const B;
    TC *const C;
    const int k;
    const int lda;
    const int ldb;
    const int ldc;
    const int ith;
    const int nth;
};
#endif // __AVX2__

} // namespace

/**
 * Performs optimized matrix multiplication on CPU.
 *
 * This subroutine may compute C = Aᵀ * B with column major ordering.
 * Despite its name, this isn't a generalized implementation. Work is
 * only performed when a handwritten kernel is written and available.
 * Otherwise the caller should fall back to a general matmul routine.
 *
 * For example, for single-threaded single-precision GEMM you can say
 *
 *     llamafile_sgemm(m, n, k, A, lda, B, ldb, C, ldc,
 *                     0, 1, GGML_TASK_TYPE_COMPUTE,
 *                     GGML_TYPE_F32, GGML_TYPE_F32, GGML_TYPE_F32);
 *
 * @param m is rows in `A` and `C`
 * @param n is cols in `B` and `C`
 * @param k is cols in `A` and rows in `B`
 * @param A is first input matrix (always transposed)
 * @param lda is row stride of `A`
 * @param B is second input matrix (never transposed)
 * @param ldb is row stride of `B`
 * @param C is input/output array of output matrices
 * @param ldc is row stride of `C`
 * @param ith is thread id (must be less than `nth`)
 * @param nth is number of threads (must be greater than zero)
 * @param task is GGML task type
 * @param Atype is GGML data type of `A`
 * @param Btype is GGML data type of `B`
 * @param Ctype is GGML data type of `C`
 * @return true if this function was able to service the matmul request
 */
bool llamafile_sgemm(int m, int n, int k, const void *A, int lda, const void *B, int ldb, void *C,
                     int ldc, int ith, int nth, int task, int Atype, int Btype, int Ctype) {

    assert(m >= 0);
    assert(n >= 0);
    assert(k >= 0);
    assert(lda >= k);
    assert(ldb >= k);
    assert(ldc >= m);
    assert(nth > 0);
    assert(ith < nth);
    assert(1ll * lda * m <= 0x7fffffff);
    assert(1ll * ldb * n <= 0x7fffffff);
    assert(1ll * ldc * n <= 0x7fffffff);

    if (Ctype != GGML_TYPE_F32)
        return false;

    switch (Atype) {

    case GGML_TYPE_F32: {
        if (Btype != GGML_TYPE_F32)
            return false;
#if defined(__AVX512F__)
        if (k % 16)
            return false;
        tinyBLAS<0, 16, __m512, __m512, float, float, float> tb{
            k, (const float *)A, lda,
            (const float *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#elif defined(__AVX__) || defined(__AVX2__)
        if (k % 8)
            return false;
        tinyBLAS<0, 8, __m256, __m256, float, float, float> tb{
            k, (const float *)A, lda,
            (const float *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#elif defined(__ARM_NEON)
        if (n < 4)
            return false;
        if (k % 4)
            return false;
        tinyBLAS<0, 4, float32x4_t, float32x4_t, float, float, float> tb{
            k, (const float *)A, lda,
            (const float *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#else
        return false;
#endif
    }

    case GGML_TYPE_F16: {
#if defined(__AVX512F__)
        if (k % 16)
            return false;
        if (Btype != GGML_TYPE_F32)
            return false;
        tinyBLAS<0, 16, __m512, __m512, ggml_fp16_t, float, float> tb{
            k, (const ggml_fp16_t *)A, lda,
            (const float *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#elif (defined(__AVX__) || defined(__AVX2__)) && defined(__F16C__)
        if (k % 8)
            return false;
        if (Btype != GGML_TYPE_F32)
            return false;
        tinyBLAS<0, 8, __m256, __m256, ggml_fp16_t, float, float> tb{
            k, (const ggml_fp16_t *)A, lda,
            (const float *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#elif defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC) && !defined(_MSC_VER)
        if (n < 8)
            return false;
        if (k % 8)
            return false;
        if (Btype != GGML_TYPE_F16)
            return false;
        tinyBLAS<0, 8, float16x8_t, float16x8_t, ggml_fp16_t, ggml_fp16_t, float> tb{
            k, (const ggml_fp16_t *)A, lda,
            (const ggml_fp16_t *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#elif defined(__ARM_NEON) && !defined(_MSC_VER)
        if (k % 4)
            return false;
        if (Btype != GGML_TYPE_F32)
            return false;
        tinyBLAS<0, 4, float32x4_t, float32x4_t, ggml_fp16_t, float, float> tb{
            k, (const ggml_fp16_t *)A, lda,
            (const float *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#else
        return false;
#endif
    }

    case GGML_TYPE_Q8_0: {
        if (Btype != GGML_TYPE_Q8_0)
           return false;
#if defined(__AVX2__) || defined(__AVX512F__)
        tinyBLAS_Q0_AVX2<0, block_q8_0, block_q8_0, float> tb{
            k, (const block_q8_0 *)A, lda,
            (const block_q8_0 *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#elif defined(__ARM_FEATURE_DOTPROD)
        tinyBLAS_Q0_ARM<0, block_q8_0> tb{
            k, (const block_q8_0 *)A, lda,
            (const block_q8_0 *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#else
        return false;
#endif
    }

    case GGML_TYPE_Q4_0: {
        if (Btype != GGML_TYPE_Q8_0)
            return false;
#if defined(__AVX2__) || defined(__AVX512F__)
        tinyBLAS_Q0_AVX2<0, block_q4_0, block_q8_0, float> tb{
            k, (const block_q4_0 *)A, lda,
            (const block_q8_0 *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#elif defined(__ARM_FEATURE_DOTPROD)
        tinyBLAS_Q0_ARM<0, block_q4_0> tb{
            k, (const block_q4_0 *)A, lda,
            (const block_q8_0 *)B, ldb,
            (float *)C, ldc,
            ith, nth};
        tb.matmul(m, n, task);
        return true;
#else
        return false;
#endif
    }

    default:
        return false;
    }

    (void)m;
    (void)n;
    (void)k;
    (void)A;
    (void)lda;
    (void)B;
    (void)ldb;
    (void)C;
    (void)ldc;
    (void)ith;
    (void)nth;
    (void)task;
    (void)Atype;
    (void)Btype;
    (void)Ctype;
}

//
//                   _   _          ___ _      _   ___
//                  | |_(_)_ _ _  _| _ ) |    /_\ / __|
//                  |  _| | ' \ || | _ \ |__ / _ \\__ \.
//                   \__|_|_||_\_, |___/____/_/ \_\___/
//                             |__/
//
//                 MIXTURE OF EXPERTS TENSOR MULTIPLICATION
//
//
// SHAPES
//
//   - weights [cols, rows, experts]
//   - thought [cols, tasks, tokens] w/ tasks ≤ thinkers
//   - result  [rows, thinkers, tokens] w/ thinkers ≤ experts
//   - plan    [thinkers, tokens] w/ i32 < experts
//
// DEFINITION
//
//   for thinker in range(thinkers):
//     for token in range(tokens):
//       for row in range(rows):
//         c = 0
//         for col in range(cols):
//           expert = plan[token][thinker]
//           a = weights[expert][row][col]
//           b = thought[token][thinker % tasks][col]
//           c += a * b
//         result[token][thinker][row] = c
//
// REGULARITIES
//
//   - tokens can be odd
//   - thinkers is usually 2
//   - tasks is usually 1 or 2
//   - cols should be a multiple of 64
//   - rows should be a multiple of 64
//   - experts is usually 8 but could be 60
//   - tokens is always 1 for token generation
//   - tokens can be huge for prompt processing
//
// EXAMPLE
//
//   mixtral 8x7b w/ 217 token prompt
//
//           |  ne*0 ne*1 ne*2 ne*3 | nb*0    nb*1      nb*2       nb*3 | type
//   =========================================================================
//   weights | 16384 6144    8    1 |   18  0x2400 0x3600000 0x1b000000 | q4_0
//   thought | 16384    2  217    1 |    4 0x10000   0x20000  0x1b20000 | f32
//   result  |  6144    2  217    1 |    4  0x6000    0xc000   0xa2c000 | f32
//   plan    |     2  217    1    1 |    4    0x20    0x1b20     0x1b20 | i32
//

namespace {
class MixMul {
  public:
    MixMul(const ggml_compute_params *params,
           const ggml_tensor *weights,
           const ggml_tensor *thought,
           const ggml_tensor *plan,
           ggml_tensor *result)
        : params(params),
          weights(weights),
          thought(thought),
          plan(plan),
          result(result),
          rows(weights->ne[1]),
          cols(weights->ne[0]),
          experts(weights->ne[2]),
          thinkers(plan->ne[0]),
          tasks(thought->ne[1]),
          tokens(thought->ne[2]),
          ldq((cols * 2 + ROW_ALIGN - 1) & -ROW_ALIGN),
          wdata_((char *)(((uintptr_t)params->wdata + MAX_ALIGN - 1) & -MAX_ALIGN)),
          allocated_(0) {
    }

    bool allocate_shared_memory() {
        if (!(quantized_thought_ = allocate<char>(MATRIX_ALIGN, tokens * tasks * ldq)))
            return false;
        if (!(rowptr_result_ = allocate<uintptr_t>(ROW_ALIGN, experts * tokens * thinkers)))
            return false;
        if (!(rowptr_thought_ = allocate<uintptr_t>(ROW_ALIGN, experts * tokens * thinkers)))
            return false;
        if (!(rowptr_count_ = allocate<int>(sizeof(int), experts)))
            return false;
        return true;
    }

    size_t get_allocated_bytes() {
        return (wdata_ - (char *)params->wdata) + allocated_;
    }

    bool mixmul() {

        // invariants
        assert(tasks <= thinkers);
        assert(thinkers <= experts);
        assert(tokens == plan->ne[1]);
        assert(rows == result->ne[0]);
        assert(cols == thought->ne[0]);
        assert(tokens == result->ne[2]);
        assert(thinkers == result->ne[1]);

        // dimensionality
        assert(plan->ne[2] == 1);
        assert(plan->ne[3] == 1);
        assert(result->ne[3] == 1);
        assert(weights->ne[3] == 1);
        assert(thought->ne[3] == 1);

        // miscellaneous
        assert(params->nth > 0);
        assert(params->ith < params->nth);
        assert(plan->type == GGML_TYPE_I32);

        // supported types
        if (result->type != GGML_TYPE_F32)
            return false;

        // check nb01 is convertible to lda
        if (weights->nb[1] % ggml_type_size(weights->type))
            return false;

        // no support for column strides
        if (result->nb[0] != ggml_type_size(result->type))
            return false;
        if (thought->nb[0] != ggml_type_size(thought->type))
            return false;
        if (weights->nb[0] != ggml_type_size(weights->type))
            return false;

        switch (weights->type) {

        case GGML_TYPE_F32:
            if (thought->type != GGML_TYPE_F32)
                return false;
#if defined(__AVX512F__)
            return mixmat<16, 1, tinyBLAS<NCB|NCC, 16, __m512, __m512,
                                          float, float, float>,
                          float, float, float>();
#elif defined(__AVX__) || defined(__AVX2__)
            return mixmat<8, 1, tinyBLAS<NCB|NCC, 8, __m256, __m256,
                                         float, float, float>,
                          float, float, float>();
#elif defined(__SSE__)
            return mixmat<4, 1, tinyBLAS<NCB|NCC, 4, __m128, __m128,
                                         float, float, float>,
                          float, float, float>();
#elif defined(__ARM_NEON)
            return mixmat<4, 1, tinyBLAS<NCB|NCC, 4, float32x4_t, float32x4_t,
                                         float, float, float>,
                          float, float, float>();
#else
            return false;
#endif

        case GGML_TYPE_F16:
            if (thought->type != GGML_TYPE_F32 &&
                thought->type != GGML_TYPE_F16)
                return false;
#if defined(__AVX512F__)
            return mixmat<16, 1, tinyBLAS<NCB|NCC, 16, __m512, __m512,
                                          ggml_fp16_t, ggml_fp16_t, float>,
                          ggml_fp16_t, ggml_fp16_t, float>();
#elif (defined(__AVX__) || defined(__AVX2__)) && defined(__F16C__)
            return mixmat<8, 1, tinyBLAS<NCB|NCC, 8, __m256, __m256,
                                         ggml_fp16_t, ggml_fp16_t, float>,
                          ggml_fp16_t, ggml_fp16_t, float>();
#elif defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC) && !defined(_MSC_VER)
            return mixmat<8, 1, tinyBLAS<NCB|NCC, 8, float16x8_t, float16x8_t,
                                         ggml_fp16_t, ggml_fp16_t, float>,
                          ggml_fp16_t, ggml_fp16_t, float>();
#elif defined(__ARM_NEON) && !defined(_MSC_VER)
            return mixmat<4, 1, tinyBLAS<NCB|NCC, 4, float32x4_t, float32x4_t,
                                         ggml_fp16_t, ggml_fp16_t, float>,
                          ggml_fp16_t, ggml_fp16_t, float>();
#else
            return false;
#endif

        case GGML_TYPE_Q4_0:
            if (thought->type != GGML_TYPE_F32 &&
                thought->type != GGML_TYPE_Q8_0)
                return false;
#if defined(__AVX2__) || defined(__AVX512F__)
            return mixmat<32, 32, tinyBLAS_Q0_AVX2<NCB|NCC, block_q4_0, block_q8_0, float>,
                          block_q4_0, block_q8_0, float>();
#elif defined(__ARM_FEATURE_DOTPROD)
            return mixmat<32, 32, tinyBLAS_Q0_ARM<NCB|NCC, block_q4_0>,
                          block_q4_0, block_q8_0, float>();
#else
            return false;
#endif

        case GGML_TYPE_Q8_0:
            if (thought->type != GGML_TYPE_F32 &&
                thought->type != GGML_TYPE_Q8_0)
                return false;
#if defined(__AVX2__) || defined(__AVX512F__)
            return mixmat<32, 32, tinyBLAS_Q0_AVX2<NCB|NCC, block_q8_0, block_q8_0, float>,
                          block_q8_0, block_q8_0, float>();
#elif defined(__ARM_FEATURE_DOTPROD)
            return mixmat<32, 32, tinyBLAS_Q0_ARM<NCB|NCC, block_q8_0>,
                          block_q8_0, block_q8_0, float>();
#else
            return false;
#endif

        default:
            return false;
        }
    }

  private:
    template <int KN, int BS, typename BLAS, typename TA, typename TB, typename TC>
    bool mixmat() {
        if (cols % KN)
            return false;
        switch (params->type) {
        case GGML_TASK_TYPE_INIT:
            if (thought->type != ggml_type_trait<TB>::id)
                quantize_thought(ggml_type_trait<TB>::id);
            build_row_pointers(ggml_type_trait<TB>::id);
            return true;
        case GGML_TASK_TYPE_COMPUTE:
            assert(!(cols % BS));
            assert(!(weights->nb[1] % sizeof(TA)));
            for (int expert = 0; expert < experts; ++expert) {
                BLAS tb{cols / BS,
                        (const TA *)((const char *)weights->data + expert*weights->nb[2]),
                        (int)(weights->nb[1] / sizeof(TA)),
                        (const TB *)(rowptr_thought_ + expert*tokens*thinkers), 0,
                        (TC *)(rowptr_result_ + expert*tokens*thinkers), 0,
                        params->ith, params->nth};
                tb.matmul(rows, rowptr_count_[expert], GGML_TASK_TYPE_COMPUTE);
            }
            return true;
        default:
            return true;
        }
    }

    void build_row_pointers(ggml_type vec_dot_type) {
        for (int expert = params->ith; expert < experts; expert += params->nth) {
            int count = 0;
            for (int token = 0; token < tokens; ++token)
                for (int thinker = 0; thinker < thinkers; ++thinker)
                    if (expert == *(const int *)((const char *)plan->data +
                                                 token*plan->nb[1] +
                                                 thinker*plan->nb[0])) {
                        int row = count++;
                        int idx = expert*thinkers*tokens + row;
                        rowptr_result_[idx] = (uintptr_t)((char *)result->data +
                                                          token*result->nb[2] +
                                                          thinker*result->nb[1]);
                        if (thought->type == vec_dot_type)
                            rowptr_thought_[idx] = (uintptr_t)((char *)thought->data +
                                                               token*thought->nb[2] +
                                                               thinker%tasks*thought->nb[1]);
                        else
                            rowptr_thought_[idx] = (uintptr_t)((char *)quantized_thought_ +
                                                               token*tasks*ldq +
                                                               thinker%tasks*ldq);
                    }
            rowptr_count_[expert] = count;
        }
    }

    void quantize_thought(ggml_type vec_dot_type) {
        int chore = 0;
        for (int token = 0; token < tokens; ++token)
            for (int task = 0; task < tasks; ++task)
                if (chore++ % params->nth == params->ith)
                    quantize_row(quantized_thought_ + token*tasks*ldq + task*ldq,
                                 (const float *)((const char *)thought->data +
                                                 token*thought->nb[2] +
                                                 task*thought->nb[1]),
                                 vec_dot_type);
    }

    void quantize_row(void *dst, const float *src, ggml_type type) {
        assert((int)ggml_row_size(type, cols) <= ldq);
        switch (type) {
        case GGML_TYPE_F16:
            ggml_fp32_to_fp16_row(src, (ggml_fp16_t *)dst, cols);
            break;
        case GGML_TYPE_Q8_0:
            quantize_row_q8_0((const float *)src, (block_q8_0 *)dst, cols);
            break;
        default:
            GGML_UNREACHABLE();
        }
    }

    template <typename T>
    T *allocate(int align, int elems) {
        T *res = nullptr;
        size_t need = sizeof(T) * elems;
        size_t base = allocated_;
        base += align - 1;
        base &= -align;
        size_t toto = base + need;
        if (toto >= allocated_ && toto <= params->wsize && elems >= 0) {
            res = (T *)(wdata_ + base);
            allocated_ = toto;
        }
        return res;
    }

    const ggml_compute_params *const params;
    const ggml_tensor *const weights;
    const ggml_tensor *const thought;
    const ggml_tensor *const plan;
    ggml_tensor *const result;
    const int rows;
    const int cols;
    const int experts;
    const int thinkers;
    const int tasks;
    const int tokens;
    const int ldq;

    // variables
    char *const wdata_;
    size_t allocated_;

    // shared memory
    int *rowptr_count_/*[experts]*/;
    char *quantized_thought_/*[tokens][tasks][cols][2]*/;
    uintptr_t *rowptr_result_/*[experts][tokens*thinkers]*/;
    uintptr_t *rowptr_thought_/*[experts][tokens*thinkers]*/;
};
} // namespace

/**
 * Performs "mixture of experts" tensor multiplication on CPU.
 */
bool llamafile_mixmul(const ggml_compute_params *params,
                      const ggml_tensor *weights,
                      const ggml_tensor *thought,
                      const ggml_tensor *plan,
                      ggml_tensor *result) {
    MixMul mm{params, weights, thought, plan, result};
    return mm.allocate_shared_memory() &&
           mm.mixmul();
}

/**
 * Returns number of shared memory bytes llamafile_mixmul() needs.
 */
size_t llamafile_mixmul_needs(const ggml_tensor *weights,
                              const ggml_tensor *thought,
                              const ggml_tensor *plan) {
    ggml_compute_params params{};
    params.wsize = 0x7ffff000;
    params.wdata = (void *)0x1000;
    MixMul mm{&params, weights, thought, plan, 0};
    if (mm.allocate_shared_memory())
        return mm.get_allocated_bytes();
    else
        return 0;
}
