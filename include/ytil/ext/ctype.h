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

#ifndef __YTIL_EXT_CTYPE_H__
#define __YTIL_EXT_CTYPE_H__

#include <ctype.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>


// return 1/0 if pred matches/not matches c
typedef int (*ctype_pred_cb)(int c);
// return replace char for c
typedef int (*ctype_transpose_cb)(int c);
// translate unit from max len bytes of src into dst, updating written and read
// is len is negative, null stop must be true, else EINVAL
// if null_stop is true also stop at '\0'
// if dst is NULL, do not write anything but update written nonetheless
// return amount of units translated or -1 on error
typedef ssize_t (*ctype_translate_cb)(unsigned char *dst, size_t *written, const unsigned char *src, size_t *read, ssize_t len, bool null_stop);

int isword(int c);      // [0-9A-Za-z_]
int issign(int c);      // [+-]
int isodigit(int c);    // [0-7]
int isbdigit(int c);    // [01]
int islxdigit(int c);   // [0-9a-z]
int isuxdigit(int c);   // [0-9A-Z]

// convert !isprint char to space
int flatten(int c);

// translate first char of src, escape with backslash if backslash or not isprint
ssize_t translate_escape(unsigned char *dst, size_t *written, const unsigned char *src, size_t *read, ssize_t len, bool null_stop);
// translate up to 4 chars of src if escaped with backslash
ssize_t translate_unescape(unsigned char *dst, size_t *writen, const unsigned char *src, size_t *read, ssize_t len, bool null_stop);

#endif
