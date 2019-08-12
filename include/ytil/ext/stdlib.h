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

#ifndef __YTIL_EXT_STDLIB_H__
#define __YTIL_EXT_STDLIB_H__

#include <stdlib.h>

// convert str of base to long int
ssize_t str2l(long int *dst, const char *str, int base);
// convert n char str prefix of base to long int
ssize_t strn2l(long int *dst, const char *str, size_t n, int base);

// convert str of base to long long int
ssize_t str2ll(long long int *dst, const char *str, int base);
// convert n char str prefix of base to long long int
ssize_t strn2ll(long long int *dst, const char *str, size_t n, int base);

// convert str of base to ssize_t (intmax_t)
ssize_t str2z(ssize_t *dst, const char *str, int base);
// convert n char str prefix of base to ssize_t (intmax_t)
ssize_t strn2z(ssize_t *dst, const char *str, size_t n, int base);

// convert str of base to unsigned long int
ssize_t str2ul(unsigned long int *dst, const char *str, int base);
// convert n char str prefix of base to unsigned long int
ssize_t strn2ul(unsigned long int *dst, const char *str, size_t n, int base);

// convert str of base to unsigned long long int
ssize_t str2ull(unsigned long long int *dst, const char *str, int base);
// convert n char str prefix of base to unsigned long long int
ssize_t strn2ull(unsigned long long int *dst, const char *str, size_t n, int base);

// convert str of base to size_t (uintmax_t)
ssize_t str2uz(size_t *dst, const char *str, int base);
// convert n char str prefix of base to size_t (uintmax_t)
ssize_t strn2uz(size_t *dst, const char *str, size_t n, int base);

// convert str to float
ssize_t str2f(float *dst, const char *str);
// convert n char str prefix to float
ssize_t strn2f(float *dst, const char *str, size_t n);
// convert n char str prefix to float, decimal point is assumed at str[n-scale]
ssize_t strs2f(float *dst, const char *str, size_t n, ssize_t scale);

// convert str to double
ssize_t str2d(double *dst, const char *str);
// convert n char str prefix to double
ssize_t strn2d(double *dst, const char *str, size_t n);
// convert n char str prefix to double, decimal point is assumed at str[n-scale]
ssize_t strs2d(double *dst, const char *str, size_t n, ssize_t scale);

// convert str to long double
ssize_t str2ld(long double *dst, const char *str);
// convert n char str prefix to long double
ssize_t strn2ld(long double *dst, const char *str, size_t n);
// convert n char str prefix to long double, decimal point is assumed at str[n-scale]
ssize_t strs2ld(long double *dst, const char *str, size_t n, ssize_t scale);

#endif
