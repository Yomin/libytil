/*
 * Copyright (c) 2019 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef __YTIL_BITS_H__
#define __YTIL_BITS_H__

#include <ytil/ext/endian.h>
#include <assert.h>


// bit value, lshift 1 'pos' times
#define BV(pos) \
    (1LLU << (pos))

// bit mask value, lshift 'mask' 'pos' times
#define BMV(mask, pos) \
    ((mask) << (pos))

// bit mask of n ones
#define BM(n) __extension__({ \
    __auto_type _n = (n); \
    _n >= sizeof(long long)*8 ? (unsigned long long)-1 : BV(_n)-1; })

// bit get, get bit at 'pos'
#define BG(value, pos) \
    (((value) >> (pos)) & 1)

// bit mask get, get mask at 'pos'
#define BMG(value, mask, pos) \
    (((value) >> (pos)) & (mask))

// byte address, get pointer to nth byte of endian dependent value
#if __BYTE_ORDER == __LITTLE_ENDIAN
    #define BA(value, n) (((unsigned char*)(value)) + (sizeof(*(value))-(n)-1))
#elif __BYTE_ORDER == __BIG_ENDIAN
    #define BA(value, n) (((unsigned char*)(value)) + (n))
#endif

// find first set
#define FFS(mask) _Generic((mask) \
    , char: __builtin_ffs(mask) \
    , unsigned char: __builtin_ffs(mask) \
    , short: __builtin_ffs(mask) \
    , unsigned short: __builtin_ffs(mask) \
    , int: __builtin_ffs(mask) \
    , unsigned int: __builtin_ffs(mask) \
    , long: __builtin_ffsl(mask) \
    , unsigned long: __builtin_ffsl(mask) \
    , long long: __builtin_ffsll(mask) \
    , unsigned long long: __builtin_ffsll(mask))

// count leading zeros
#define CLZ(mask) _Generic((mask) \
    , char: __builtin_clz(mask)-24 \
    , unsigned char: __builtin_clz(mask)-24 \
    , short: __builtin_clz(mask)-16 \
    , unsigned short: __builtin_clz(mask)-16 \
    , int: __builtin_clz(mask) \
    , unsigned int: __builtin_clz(mask) \
    , long: __builtin_clzl(mask) \
    , unsigned long: __builtin_clzl(mask) \
    , long long: __builtin_clzll(mask) \
    , unsigned long long: __builtin_clzll(mask))

// count trailing zeros
#define CTZ(mask) _Generic((mask) \
    , char: __builtin_ctz(mask) \
    , unsigned char: __builtin_ctz(mask) \
    , short: __builtin_ctz(mask) \
    , unsigned short: __builtin_ctz(mask) \
    , int: __builtin_ctz(mask) \
    , unsigned int: __builtin_ctz(mask) \
    , long: __builtin_ctzl(mask) \
    , unsigned long: __builtin_ctzl(mask) \
    , long long: __builtin_ctzll(mask) \
    , unsigned long long: __builtin_ctzll(mask))


#endif
