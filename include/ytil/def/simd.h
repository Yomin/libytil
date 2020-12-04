/*
 * Copyright (c) 2019-2020 Martin Rödel a.k.a. Yomin Nimoy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/// \file

#ifndef YTIL_DEF_SIMD_H_INCLUDED
#define YTIL_DEF_SIMD_H_INCLUDED


#if defined(__MMX__) && defined(__SSE__)
    #include <mmintrin.h>
    #include <xmmintrin.h>
    #define SIMD    1
    #define SIMD64  1
#endif

#if SIMD || DOXYGEN

#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512BW__)
    #include <immintrin.h>
    #define SIMD128 1
    #define SIMD256 1
    #define SIMD512 1
#endif

#if !defined(SIMD128) && defined(__SSE2__)
    #include <emmintrin.h>
    #define SIMD128 1
#endif

#if !defined(SIMD256) && defined(__AVX__) && defined(__AVX2__)
    #include <immintrin.h>
    #define SIMD256 1
#endif


#include <ytil/def/bits.h>
#include <stdint.h>


/// Find index of 8 bit key in 64 bits of data.
///
/// \param data     data to search
/// \param size     number of actual keys in data
/// \param key      key to find
///
/// \returns        index of key
/// \retval -1      key not found
extern inline int simd64_index8(const void *data, unsigned int size, int8_t key)
{
    __m64 cmp;
    int mask;

    cmp     = _mm_cmpeq_pi8(_mm_set1_pi8(key), *(__m64 *)data);
    mask    = _mm_movemask_pi8(cmp) & BM(size);

    return mask ? CTZ(mask) : -1;
}

/// Find index of 8 bit key in 128 bits of data.
///
/// \param data     data to search
/// \param size     number of actual keys in data
/// \param key      key to find
///
/// \returns        index of key
/// \retval -1      key not found
extern inline int simd128_index8(const void *data, unsigned int size, int8_t key)
{
#if SIMD512

    __mmask16 mask;

    mask    = _mm_cmpeq_epi8_mask(_mm_set1_epi8(key), _mm_loadu_si128(data));
    mask    &= BM(size);

    return mask ? CTZ(mask) : -1;

#elif SIMD128

    __m128i cmp;
    int mask;

    cmp     = _mm_cmpeq_epi8(_mm_set1_epi8(key), _mm_loadu_si128(data));
    mask    = _mm_movemask_epi8(cmp) & BM(size);

    return mask ? CTZ(mask) : -1;

#else // if SIMD512

    int index;

    if((index = simd64_index8(data, size, key)) >= 0)
        return index;

    if(size > 8 && (index = simd64_index8(((unsigned char *)data) + 8, size - 8, key)) >= 0)
        return index + 8;

    return -1;

#endif // if SIMD512
}

/// Find index of 8 bit key in 256 bits of data.
///
/// \param data     data to search
/// \param size     number of actual keys in data
/// \param key      key to find
///
/// \returns        index of key
/// \retval -1      key not found
extern inline int simd256_index8(const void *data, unsigned int size, int8_t key)
{
#if SIMD512

    __mmask32 mask;

    mask    = _mm256_cmpeq_epi8_mask(_mm256_set1_epi8(key), _mm_loadu_si256(data));
    mask    &= BM(size);

    return mask ? CTZ(mask) : -1;

#elif SIMD256

    __m256i cmp;
    int mask;

    cmp     = _mm256_cmpeq_epi8(_mm256_set1_epi8(key), _mm256_loadu_si256(data));
    mask    = _mm256_movemask_epi8(cmp) & BM(size);

    return mask ? CTZ(mask) : -1;

#else // if SIMD512

    int index;

    if((index = simd128_index8(data, size, key)) >= 0)
        return index;

    if(size > 16 && (index = simd128_index8(((unsigned char *)data) + 16, size - 16, key)) >= 0)
        return index + 16;

    return -1;

#endif // if SIMD512
}

/// Find index of 8 bit key in 512 bits of data.
///
/// \param data     data to search
/// \param size     number of actual keys in data
/// \param key      key to find
///
/// \returns        index of key
/// \retval -1      key not found
extern inline int simd512_index8(const void *data, unsigned int size, int8_t key)
{
#if SIMD512

    __mmask64 mask;

    mask    = _mm512_cmpeq_epi8_mask(_mm512_set1_epi8(key), _mm512_loadu_si512(data));
    mask    &= BM(size);

    return mask ? CTZ(mask) : -1;

#else

    int index;

    if((index = simd256_index8(data, size, key)) >= 0)
        return index;

    if(size > 32 && (index = simd256_index8(((unsigned char *)data) + 32, size - 32, key)) >= 0)
        return index + 32;

    return -1;

#endif // if SIMD512
}

/// Find index of 8 bit key in 1024 bits of data.
///
/// \param data     data to search
/// \param size     number of actual keys in data
/// \param key      key to find
///
/// \returns        index of key
/// \retval -1      key not found
extern inline int simd1024_index8(const void *data, unsigned int size, int8_t key)
{
    int index;

    if((index = simd512_index8(data, size, key)) >= 0)
        return index;

    if(size > 64 && (index = simd512_index8(((unsigned char *)data) + 64, size - 64, key)) >= 0)
        return index + 64;

    return -1;
}

#endif // if SIMD || DOXYGEN

#endif // ifndef YTIL_DEF_SIMD_H_INCLUDED
