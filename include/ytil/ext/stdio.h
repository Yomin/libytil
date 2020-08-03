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

#ifndef YTIL_EXT_STDIO_H_INCLUDED
#define YTIL_EXT_STDIO_H_INCLUDED

#include <ytil/ext/alloca.h>
#include <stdio.h>
#include <stdarg.h>

// format string into stack allocated buffer
#define FMT(fmt, ...) __extension__ ({ \
    char *data; \
    int len; \
    \
    len  = snprintf(NULL, 0, fmt, __VA_ARGS__); \
    data = alloca(len+1); \
    snprintf(data, len+1, fmt, __VA_ARGS__); \
    \
    data; })

// format string into stack allocated buffer, va_list version
#define VFMT(fmt, ap1) __extension__ ({ \
    va_list ap2; \
    char *data; \
    int len; \
    \
    va_copy(ap2, ap1); \
    len = vsnprintf(NULL, 0, fmt, ap2); \
    va_end(ap2); \
    \
    data = alloca(len+1); \
    vsnprintf(data, len+1, fmt, ap1); \
    \
    data; })

// format string into stack allocated buffer, implicit use of variable arguments
#define VVFMT(fmt) __extension__ ({ \
    va_list ap1, ap2; \
    char *data; \
    int len; \
    \
    va_start(ap1, fmt); \
    va_copy(ap2, ap1); \
    len = vsnprintf(NULL, 0, fmt, ap2); \
    va_end(ap2); \
    \
    data = alloca(len+1); \
    vsnprintf(data, len+1, fmt, ap1); \
    va_end(ap1); \
    \
    data; })


// format string into allocated buffer
char *strdup_printf(const char *fmt, ...) __attribute__((format (gnu_printf, 1, 2)));
// format string into allocated buffer, va_list version
char *strdup_vprintf(const char *fmt, va_list ap) __attribute__((format (gnu_printf, 1, 0)));

// prints hex encoded memory to stdout
void dump(void *mem, size_t size);
// prints hex encoded memory to stream
void fdump(FILE *fp, void *mem, size_t size);

#endif
