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

#ifndef YTIL_DEF_BITS_H_INCLUDED
#define YTIL_DEF_BITS_H_INCLUDED

#include <ytil/ext/endian.h>
#include <stdint.h>


/// Bit value.
///
/// \param pos      number of times to left shift 1
#define BV(pos) \
    (((uintmax_t)1) << (pos))

/// Bit mask value.
///
/// \param mask     mask to shift
/// \param pos      number of times to left shift mask
#define BMV(mask, pos) \
    ((mask) << (pos))

/// Bit mask of n ones.
///
/// \param n    number of ones
#define BM(n) __extension__ ({                              \
    uintmax_t _n = (n);                                     \
    _n >= sizeof(uintmax_t)*8 ? (uintmax_t)-1 : BV(_n) - 1; \
})

/// Bit get.
///
/// \param value    value to get bit from
/// \param pos      position to get bit at
#define BG(value, pos) \
    (((value) >> (pos)) & 1U)

/// Bit mask get.
///
/// \param value    value to get mask from
/// \param mask     mask to get
/// \param pos      position to get mask at
#define BMG(value, mask, pos) \
    (((value) >> (pos)) & (mask))

/// Byte address. Get pointer to nth byte of endian dependent value.
///
/// \param value    value to get nth byte from
/// \param pos      position to get byte at
#if __BYTE_ORDER == __LITTLE_ENDIAN
    #define BA(value, pos) (((unsigned char *)(value)) + (sizeof(*(value)) - (pos) - 1))
#elif __BYTE_ORDER == __BIG_ENDIAN
    #define BA(value, pos) (((unsigned char *)(value)) + (pos))
#endif

/// Find first set.
///
/// \param mask     mask to find first set bit in
#define FFS(mask) _Generic((mask)                 \
    , char:                 __builtin_ffs(mask)   \
    , unsigned char:        __builtin_ffs(mask)   \
    , short:                __builtin_ffs(mask)   \
    , unsigned short:       __builtin_ffs(mask)   \
    , int:                  __builtin_ffs(mask)   \
    , unsigned int:         __builtin_ffs(mask)   \
    , long:                 __builtin_ffsl(mask)  \
    , unsigned long:        __builtin_ffsl(mask)  \
    , long long:            __builtin_ffsll(mask) \
    , unsigned long long:   __builtin_ffsll(mask))

/// Count leading zeros.
///
/// \param mask     mask to count leading zeros in
#define CLZ(mask) _Generic((mask)                    \
    , char:                 __builtin_clz(mask) - 24 \
    , unsigned char:        __builtin_clz(mask) - 24 \
    , short:                __builtin_clz(mask) - 16 \
    , unsigned short:       __builtin_clz(mask) - 16 \
    , int:                  __builtin_clz(mask)      \
    , unsigned int:         __builtin_clz(mask)      \
    , long:                 __builtin_clzl(mask)     \
    , unsigned long:        __builtin_clzl(mask)     \
    , long long:            __builtin_clzll(mask)    \
    , unsigned long long:   __builtin_clzll(mask))

/// Count trailing zeros.
///
/// \param mask     mask to count trailing zeros in
#define CTZ(mask) _Generic((mask)                 \
    , char:                 __builtin_ctz(mask)   \
    , unsigned char:        __builtin_ctz(mask)   \
    , short:                __builtin_ctz(mask)   \
    , unsigned short:       __builtin_ctz(mask)   \
    , int:                  __builtin_ctz(mask)   \
    , unsigned int:         __builtin_ctz(mask)   \
    , long:                 __builtin_ctzl(mask)  \
    , unsigned long:        __builtin_ctzl(mask)  \
    , long long:            __builtin_ctzll(mask) \
    , unsigned long long:   __builtin_ctzll(mask))


#endif // ifndef YTIL_DEF_BITS_H_INCLUDED
