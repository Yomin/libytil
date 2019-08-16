/*
 * Copyright (c) 2017-2019 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef __YTIL_DEF_H__
#define __YTIL_DEF_H__

#include <ytil/gen/error.h>
#include <stdlib.h>
#include <endian.h>
#include <assert.h>


// return the smallest of two values
#define MIN(a, b) __extension__({ \
    __auto_type _a = (a); \
    __auto_type _b = (b); \
    _a < _b ? _a : _b; })

// return the smallest of three values
#define MIN3(a, b, c) __extension__({ \
    __auto_type _a = (a); \
    __auto_type _b = (b); \
    __auto_type _c = (c); \
    _a < _b ? (_a < _c ? _a : _c) : (_b < _c ? _b : _c); })

// return the largest of two values
#define MAX(a, b) __extension__({ \
    __auto_type _a = (a); \
    __auto_type _b = (b); \
    _a > _b ? _a : _b; })

// return the largest of three values
#define MAX3(a, b, c) __extension__({ \
    __auto_type _a = (a); \
    __auto_type _b = (b); \
    __auto_type _c = (c); \
    _a > _b ? (_a > _c ? _a : _c) : (_b > _c ? _b : _c); })

// return the absolute value
#define ABS(a) __extension__({ \
    __auto_type _a = (a); \
    _a < 0 ? -_a : _a; })

// check if n in range min <= n <= max
#define RANGE(n, min, max) __extension__({ \
    __auto_type _n = (n); \
    __auto_type _min = (min); \
    __auto_type _max = (max); \
    _min <= _n && _n <= _max; })

// clamp value to range min <= n <= max
#define CLAMP(n, min, max) __extension__({ \
    __auto_type _n = (n); \
    __auto_type _min = (min); \
    __auto_type _max = (max); \
    _n < _min ? _min : _n > _max ? _max : _n; })

// return number of static array elements
#define ELEMS(array) \
    (sizeof(array) / sizeof(array[0]))

// suppress compiler warning for unused variable
#define UNUSED(x) \
    (void)(x)

// bit value, lshift 1 'pos' times
#define BV(pos) \
    (1 << (pos))

// bit mask value, lshift 'mask' 'pos' times
#define BMV(mask, pos) \
    ((m) << (pos))

// bit mask of n ones
#define BM(n) \
    (BV(n)-1)

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

// cast value to type, eg value or function pointer to void*
#define CAST(type, value) __extension__({ \
    assert(sizeof(type) >= sizeof(intptr_t)); \
    (type)(intptr_t)(value); })

// return if !condition
#define return_if_fail(condition) do { \
    if(!(condition)) \
        return; \
} while(0)

// return value if !condition
#define return_value_if_fail(condition, value) do { \
    if(!(condition)) \
        return (value); \
} while(0)

// set error and return value if !condition
#define return_error_if_fail(condition, error, value) do { \
    if(!(condition)) \
    { \
        error_set(error); \
        return (value); \
    } \
} while(0)

// set error and return value if reached
#define return_error_if_reached(error, value) do { \
    error_set(error); \
    return (value); \
} while(0)

#ifdef NDEBUG
    
    // does nothing in production mode
    #define DEBUG(x)
    
    // insert production value
    #define PROD_DEBUG(p, d) p
    
#else
    
    // insert x in debug mode
    #define DEBUG(x) x
    
    // insert debug value
    #define PROD_DEBUG(p, d) d
    
#endif

#endif