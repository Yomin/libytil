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

#endif
