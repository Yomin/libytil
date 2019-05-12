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

#ifndef __YTIL_EXT_STRING_H__
#define __YTIL_EXT_STRING_H__

#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

#include <ytil/ext/ctype.h>

// duplicate string (system provided)
// char *strdup(const char *str);
// duplicate memory, no deep copy
void *memdup(const void *mem, size_t size);

// fill dst with pattern
void *mempat(void *dst, size_t len, const void *pat, size_t patlen);

// copy string (system provided)
// char *strcpy(char *dst, const char *src);
// copy memory (system provided)
// void *memcpy(void *dst, const void *src, size_t size);
// reverse memcpy
void *memrcpy(void *dst, const void *src, size_t size);

// scan str for first occurence of char (system provided)
// char *strchr(const char *str, int c);
// scan str for first occurence of char, check max first n chars
char *strnchr(const char *str, int c, size_t n);
// scan str for first occurence of char, check max first n chars, decrease n
char *strpnchr(const char *str, int c, size_t *n);

// scan mem first occurence of char (system provided)
// void *memchr(const void *mem, int c, size_t size);

// scan str for first occurence of any char in accept (system provided)
// char *strpbrk(const char *str, const char *accept);
// scan str for last occurence of any char in accept
char *strrpbrk(const char *str, const char *accept);
// scan str for first occurence of any char not in reject
char *strcpbrk(const char *str, const char *reject);
// scan str for last occurence of any char not in reject
char *strrcpbrk(const char *str, const char *reject);

// scan mem for first occurence of any byte in accept
void *mempbrk(const void *mem, size_t msize, const void *accept, size_t asize);
// scan mem for last occurence of any byte in accept
void *memrpbrk(const void *mem, size_t msize, const void *accept, size_t asize);
// scan mem for first occurence of any byte not in reject
void *memcpbrk(const void *mem, size_t msize, const void *reject, size_t rsize);
// scan mem for last occurence of any byte not in reject
void *memrcpbrk(const void *mem, size_t msize, const void *reject, size_t rsize);

// scan str for first occurence of a char not satisfying pred
char *strwhile(const char *str, ctype_pred_cb pred);
// scan str for first occurence of a char not satisfying pred, check max first n chars
char *strnwhile(const char *str, ctype_pred_cb pred, size_t n);
// scan str for first occurence of a char not satisfying pred, check max first n chars, decrease n
char *strpnwhile(const char *str, ctype_pred_cb pred, size_t *n);
// scan str for last occurence of a char not satisfying pred
char *strrwhile(const char *str, ctype_pred_cb pred);
// scan str for last occurence of a char not satisfying pred, check max last n chars
char *strnrwhile(const char *str, ctype_pred_cb pred, size_t n);

// scan str for first occurence of a char satisfying pred
char *struntil(const char *str, ctype_pred_cb pred);
// scan str for first occurence of a char satisfying pred, check max first n chars
char *strnuntil(const char *str, ctype_pred_cb pred, size_t n);
// scan str for first occurence of a char satisfying pred, check max first n chars, decrease n
char *strpnuntil(const char *str, ctype_pred_cb pred, size_t *n);
// scan str for last occurence of a char satisfying pred
char *strruntil(const char *str, ctype_pred_cb pred);
// scan str for last occurence of a char satisfying pred, check max last n chars
char *strnruntil(const char *str, ctype_pred_cb pred, size_t n);

// scan mem for first occurence of a byte not satisfying pred
void *memwhile(const void *mem, size_t size, ctype_pred_cb pred);
// scan mem for last occurence of a byte not satisfying pred
void *memrwhile(const void *mem, size_t size, ctype_pred_cb pred);
// scan mem for first occurence of a byte satisfying pred
void *memuntil(const void *mem, size_t size, ctype_pred_cb pred);
// scan mem for last occurence of a byte satisfying pred
void *memruntil(const void *mem, size_t size, ctype_pred_cb pred);

// strwhile(str, (==c))
char *strskip(const char *str, int c);
// strrwhile(str, (==c))
char *strrskip(const char *str, int c);

// memwhile(str, size, (==c))
void *memskip(const void *mem, size_t size, int c);
// memrwhile(str, size, (==c))
void *memrskip(const void *mem, size_t size, int c);

// get length of prefix with chars in accept (system provided)
// size_t strspn(const char *str, const char *accept);
// get length of prefix (max n chars) with chars in accept
size_t strnspn(const char *str, const char *accept, size_t n);
// get length of prefix with chars not in reject (system provided)
// size_t strcspn(const char *str, const char *reject);
// get length of prefix (max n chars) with chars not in reject
size_t strncspn(const char *str, const char *reject, size_t n);

// get length of prefix with bytes in accept
size_t memspn(const void *mem, size_t msize, const void *accept, size_t asize);
// get length of prefix with bytes not in reject
size_t memcspn(const void *mem, size_t msize, const void *reject, size_t rsize);

// get length of common prefix
size_t strprefix(const char *str1, const char *str2);
// get length of common prefix ignoring case
size_t strcaseprefix(const char *str1, const char *str2);
// get length of common prefix, limited to len
size_t strnprefix(const char *str1, const char *str2, size_t len);
// get length of common prefix ignoring case, limited to len
size_t strncaseprefix(const char *str1, const char *str2, size_t len);

// get length of common prefix
size_t memprefix(const void *mem1, const void *mem2, size_t size);
// get length of common prefix ignoring case (where applicable)
size_t memcaseprefix(const void *mem1, const void *mem2, size_t size);

// scan str haystack for first occurence of str needle (system provided)
// char *strstr(const char *haystack, const char *needle);
// scan mem haystack first occurence of mem needle
void *memmem(const void *haystack, size_t ssize, const void *needle, size_t nsize);

// compare two strings (system provided)
// int strcmp(const char *str1, const char *str2);
// compare two strings ignoring case (system provided)
// int strcasecmp(const char *str1, const char *str2);

// compare two memory areas (system provided)
// int memcmp(const void *mem1, const void *mem2, size_t size);
// compare two memory areas ignoring case (where applicable)
int memcasecmp(const void *mem1, const void *mem2, size_t size);

// remove any chars in reject trailing str left or right, str is modified
char *trim(char *str, const char *reject);
// remove any chars satisfying pred trailing str left or right, str is modified
char *trim_pred(char *str, ctype_pred_cb pred);
// trim_pred(str, isblank)
char *trim_blank(char *str);
// trim_pred(str, isspace)
char *trim_space(char *str);

// remove any chars in reject trailing str on the right, str is modified
char *rtrim(char *str, const char *reject);
// remove any chars satisfying pred trailing str on the right, str is modified
char *rtrim_pred(char *str, ctype_pred_cb pred);
// rtrim_pred(str, isblank)
char *rtrim_blank(char *str);
// rtrim_pred(str, isspace)
char *rtrim_space(char *str);

// remove any bytes in reject trailing mem left or right, msize is modified
void *memtrim(const void *mem, size_t *msize, const void *reject, size_t rsize);
// remove any bytes satisfying pred trailing mem left or right, size is modified
void *memtrim_pred(const void *mem, size_t *size, ctype_pred_cb pred);
// remove any bytes in reject trailing mem on the right, msize is modified
void *memrtrim(const void *mem, size_t *msize, const void *reject, size_t rsize);
// remove any bytes satisfying pred trailing mem on the right, size is modified
void *memrtrim_pred(const void *mem, size_t *size, ctype_pred_cb pred);

// convert every char in str to uppercase
char *strupper(char *str);
// convert max n chars in str to uppercase
char *strnupper(char *str, size_t n);
// convert every char in mem to uppercase (where applicable)
void *memupper(void *mem, size_t size);

// convert every char in str to lowercase
char *strlower(char *str);
// convert max n chars in str to lowercase
char *strnlower(char *str, size_t n);
// convert every char in mem to lowercase (where applicable)
void *memlower(void *mem, size_t size);

// convert every non-print char in str to space
char *strflat(char *str);
// convert every non-print char in first n chars of str to space
char *strnflat(char *str, size_t n);
// convert every non-print char in mem to space
void *memflat(void *mem, size_t size);

// transpose chars from str occuring in 'from' with chars in 'to' using the same index
char *strtrans(char *str, const char *from, const char *to);
// transpose bytes from mem occuring in 'from' with bytes in 'to' using the same index
void *memtrans(void *mem, size_t msize, const void *from, const void *to, size_t tsize);

// escape all chars not satisfying 'keep' with 'esc' into dst
size_t strescape(char *dst, const char *src, int esc, ctype_pred_cb keep);
// unescape all chars preceeded by 'esc'
ssize_t strunescape(char *dst, const char *src, int esc);
// strescape into allocated buffer
char *strdup_escape(const char *src, int esc, ctype_pred_cb keep);
// strunescape into allocated buffer
char *strdup_unescape(const char *src, int esc);

#endif
