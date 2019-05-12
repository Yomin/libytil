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

#include "stdlib.h"
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <locale.h>


ssize_t str2l(long int *dst, const char *str, int base)
{
    long int tmp;
    char *end;
    
    errno = 0;
    dst = dst ? dst : &tmp;
    *dst = strtol(str, &end, base);
    
    if(errno)
        return -1;
    
    if(end[0])
        return errno = EINVAL, -1;
    
    return end - str;
}

ssize_t strn2l(long int *dst, const char *str, size_t n, int base)
{
    char *tmp = alloca(n+1);
    
    memcpy(tmp, str, n);
    tmp[n] = '\0';
    
    return str2l(dst, tmp, base);
}

ssize_t str2ll(long long int *dst, const char *str, int base)
{
    long long int tmp;
    char *end;
    
    errno = 0;
    dst = dst ? dst : &tmp;
    *dst = strtoll(str, &end, base);
    
    if(errno)
        return -1;
    
    if(end[0])
        return errno = EINVAL, -1;
    
    return end - str;
}

ssize_t strn2ll(long long int *dst, const char *str, size_t n, int base)
{
    char *tmp = alloca(n+1);
    
    memcpy(tmp, str, n);
    tmp[n] = '\0';
    
    return str2ll(dst, tmp, base);
}

ssize_t str2z(ssize_t *dst, const char *str, int base)
{
    intmax_t tmp;
    char *end;
    
    errno = 0;
    dst = dst ? dst : &tmp;
    *dst = strtoimax(str, &end, base);
    
    if(errno)
        return -1;
    
    if(end[0])
        return errno = EINVAL, -1;
    
    return end - str;
}

ssize_t strn2z(ssize_t *dst, const char *str, size_t n, int base)
{
    char *tmp = alloca(n+1);
    
    memcpy(tmp, str, n);
    tmp[n] = '\0';
    
    return str2z(dst, tmp, base);
}

ssize_t str2ul(unsigned long int *dst, const char *str, int base)
{
    unsigned long int tmp;
    char *end;
    
    errno = 0;
    dst = dst ? dst : &tmp;
    *dst = strtoul(str, &end, base);
    
    if(errno)
        return -1;
    
    if(end[0])
        return errno = EINVAL, -1;
    
    return end - str;
}

ssize_t strn2ul(unsigned long int *dst, const char *str, size_t n, int base)
{
    char *tmp = alloca(n+1);
    
    memcpy(tmp, str, n);
    tmp[n] = '\0';
    
    return str2ul(dst, tmp, base);
}

ssize_t str2ull(unsigned long long int *dst, const char *str, int base)
{
    unsigned long long int tmp;
    char *end;
    
    errno = 0;
    dst = dst ? dst : &tmp;
    *dst = strtoull(str, &end, base);
    
    if(errno)
        return -1;
    
    if(end[0])
        return errno = EINVAL, -1;
    
    return end - str;
}

ssize_t strn2ull(unsigned long long int *dst, const char *str, size_t n, int base)
{
    char *tmp = alloca(n+1);
    
    memcpy(tmp, str, n);
    tmp[n] = '\0';
    
    return str2ull(dst, tmp, base);
}

ssize_t str2uz(size_t *dst, const char *str, int base)
{
    uintmax_t tmp;
    char *end;
    
    errno = 0;
    dst = dst ? dst : &tmp;
    *dst = strtoumax(str, &end, base);
    
    if(errno)
        return -1;
    
    if(end[0])
        return errno = EINVAL, -1;
    
    return end - str;
}

ssize_t strn2uz(size_t *dst, const char *str, size_t n, int base)
{
    char *tmp = alloca(n+1);
    
    memcpy(tmp, str, n);
    tmp[n] = '\0';
    
    return str2uz(dst, tmp, base);
}

static inline char *scalecopy(char *buf, const char *str, size_t n, ssize_t scale)
{
    size_t dot;
    
    assert(scale <= n);
    
    if(scale <= 0)
    {
        memcpy(buf, str, n);
        buf[n] = '\0';
    }
    else
    {
        dot = n - scale;
        memcpy(buf, str, dot);
        buf[dot] = localeconv()->decimal_point[0];
        memcpy(buf+dot+1, str+dot, scale);
        buf[n+1] = '\0';
    }
    
    return buf;
}

ssize_t str2f(float *dst, const char *str)
{
    float tmp;
    char *end;
    
    errno = 0;
    dst = dst ? dst : &tmp;
    *dst = strtof(str, &end);
    
    if(errno)
        return -1;
    
    if(end[0])
        return errno = EINVAL, -1;
    
    return end - str;
}

ssize_t strn2f(float *dst, const char *str, size_t n)
{
    return str2f(dst, scalecopy(alloca(n+2), str, n, -1));
}

ssize_t strs2f(float *dst, const char *str, size_t n, ssize_t scale)
{
    return str2f(dst, scalecopy(alloca(n+2), str, n, scale));
}

ssize_t str2d(double *dst, const char *str)
{
    double tmp;
    char *end;
    
    errno = 0;
    dst = dst ? dst : &tmp;
    *dst = strtod(str, &end);
    
    if(errno)
        return -1;
    
    if(end[0])
        return errno = EINVAL, -1;
    
    return end - str;
}

ssize_t strn2d(double *dst, const char *str, size_t n)
{
    return str2d(dst, scalecopy(alloca(n+2), str, n, -1));
}

ssize_t strs2d(double *dst, const char *str, size_t n, ssize_t scale)
{
    return str2d(dst, scalecopy(alloca(n+2), str, n, scale));
}

ssize_t str2ld(long double *dst, const char *str)
{
    long double tmp;
    char *end;
    
    errno = 0;
    dst = dst ? dst : &tmp;
    *dst = strtold(str, &end);
    
    if(errno)
        return -1;
    
    if(end[0])
        return errno = EINVAL, -1;
    
    return end - str;
}

ssize_t strn2ld(long double *dst, const char *str, size_t n)
{
    return str2ld(dst, scalecopy(alloca(n+2), str, n, -1));
}

ssize_t strs2ld(long double *dst, const char *str, size_t n, ssize_t scale)
{
    return str2ld(dst, scalecopy(alloca(n+2), str, n, scale));
}
