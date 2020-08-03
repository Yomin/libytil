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
#ifdef _WIN32
// duplicate first n chars of str
char *strndup(const char *str, size_t n);
#endif
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
// scan str for first occurence of any char in accept, check max first n chars
char *strnpbrk(const char *str, const char *accept, size_t n);
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

// count occurences of c in str
size_t strcnt(const char *str, int c);
// count occurences of c in max n chars of str
size_t strncnt(const char *str, int c, size_t n);
// count occurences of c in mem
size_t memcnt(const void *mem, size_t size, int c);

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

// check if 'prefix' is prefix of str
bool strprefix(const char *str, const char *prefix);
// check if 'prefix' is prefix of str ignoring case
bool strcaseprefix(const char *str, const char *prefix);
// check if 'prefix' of length plen is prefix of str of length slen
bool strnprefix(const char *str, size_t slen, const char *prefix, size_t plen);
// check if 'prefix' of length plen is prefix of str of length slen ignoring case
bool strncaseprefix(const char *str, size_t slen, const char *prefix, size_t plen);

// check if 'prefix' is prefix of mem
bool memprefix(const void *mem, size_t msize, const void *prefix, size_t psize);
// check if 'prefix' is prefix of mem ignoring case (where applicable)
bool memcaseprefix(const void *mem, size_t msize, const void *prefix, size_t psize);

// get length of common prefix
size_t strprefixlen(const char *str1, const char *str2);
// get length of common prefix ignoring case
size_t strcaseprefixlen(const char *str1, const char *str2);
// get length of common prefix, limited by MIN(slen1, slen2)
size_t strnprefixlen(const char *str1, size_t slen1, const char *str2, size_t slen2);
// get length of common prefix ignoring case, limited by MIN(slen1, slen2)
size_t strncaseprefixlen(const char *str1, size_t slen1, const char *str2, size_t slen2);

// get length of common prefix
size_t memprefixlen(const void *mem1, size_t size1, const void *mem2, size_t size2);
// get length of common prefix ignoring case (where applicable)
size_t memcaseprefixlen(const void *mem1, size_t size1, const void *mem2, size_t size2);

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

// transpose chars from str occuring in 'from' with chars in 'to' using the same index
char *strtranspose(char *str, const char *from, const char *to);
// transpose chars from str by appying trans to every character
char *strtranspose_f(char *str, ctype_transpose_cb trans);
// convert every char in str to uppercase
char *strtranspose_upper(char *str);
// convert max n chars in str to uppercase
char *strtranspose_upper_n(char *str, size_t n);
// convert every char in str to lowercase
char *strtranspose_lower(char *str);
// convert max n chars in str to lowercase
char *strtranspose_lower_n(char *str, size_t n);
// convert every non-print char in str to space
char *strtranspose_flatten(char *str);
// convert every non-print char in first n chars of str to space
char *strtranspose_flatten_n(char *str, size_t n);

// transpose bytes from mem occuring in 'from' with bytes in 'to' using the same index
void *memtranspose(void *mem, size_t msize, const void *from, const void *to, size_t tsize);
// transpose bytes from mem by appying trans to every byte
void *memtranspose_f(void *mem, size_t size, ctype_transpose_cb trans);
// convert every char in mem to uppercase (where applicable)
void *memtranspose_upper(void *mem, size_t size);
// convert every char in mem to lowercase (where applicable)
void *memtranspose_lower(void *mem, size_t size);
// convert every non-print char in mem to space
void *memtranspose_flatten(void *mem, size_t size);


// translate src string into dst string using trans
// returns length of translated string, dst must have one extra byte for terminator
ssize_t strtranslate(char *dst, const char *src, ctype_translate_cb trans);
// translate max n chars of src string into dst string using trans
// returns length of translated string, dst must have one extra byte for terminator
ssize_t strtranslate_n(char *dst, const char *src, ctype_translate_cb trans, size_t n);
// translate src memory into dst string using trans
// returns length of translated string, dst must have one extra byte for terminator
ssize_t strtranslate_mem(char *dst, const void *src, size_t size, ctype_translate_cb trans);

// translate src memory into dst memory using trans
// returns length of translated memory
ssize_t memtranslate(void *dst, const void *src, size_t size, ctype_translate_cb trans);
// translate src string into dst memory using trans
// returns length of translated memory
ssize_t memtranslate_str(void *dst, const char *src, ctype_translate_cb trans);
// translate max n chars of src string into dst memory using trans
// returns length of translated memory
ssize_t memtranslate_str_n(void *dst, const char *src, ctype_translate_cb trans, size_t n);

// translate src into dst escaping all backslash or !isprint chars
// returns length of escaped str, dst must have one extra byte for terminator
size_t strescape(char *dst, const char *src);
// translate max n chars of src into dst escaping all backslash or !isprint chars
// returns length of escaped str, dst must have one extra byte for terminator
size_t strescape_n(char *dst, const char *src, size_t n);
// translate src into dst escaping all backslash or !isprint chars
// returns length of escaped str, dst must have one extra byte for terminator
size_t strescape_mem(char *dst, const void *src, size_t size);
// translate src into dst unescaping all chars preceeded by backslash
// returns length of unescaped str, dst must have one extra byte for terminator
ssize_t strunescape(char *dst, const char *src);
// translate max n chars of src into dst unescaping all chars preceeded by backslash
// returns length of unescaped str, dst must have one extra byte for terminator
ssize_t strunescape_n(char *dst, const char *src, size_t n);
// translate src into dst unescaping all chars preceeded by backslash
// returns length of unescaped memory
ssize_t strunescape_mem(void *dst, const char *src);
// translate max n chars of src into dst unescaping all chars preceeded by backslash
// returns length of unescaped memory
ssize_t strunescape_mem_n(void *dst, const char *src, size_t n);

#endif
