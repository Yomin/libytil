/*
 * Copyright (c) 2018-2019 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_EXT_GETOPT_H_INCLUDED
#define YTIL_EXT_GETOPT_H_INCLUDED

#include <getopt.h>

typedef struct option getopt_option_st;

typedef int (*getopt_short_cb)(int opt, char *arg, void *ctx);
typedef int (*getopt_long_cb)(int idx, int opt, char *arg, void *ctx);

// remove matching short options from argc/argv and call callback on them
int getopt_sieve(int *argc, char *argv[], const char *opts, getopt_short_cb cb, void *ctx);
// remove matching long options from argc/argv and call callback on them
int getopt_long_sieve(int *argc, char *argv[], const getopt_option_st *opts, getopt_long_cb cb, void *ctx);

#endif
