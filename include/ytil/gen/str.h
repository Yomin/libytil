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

#ifndef __YTIL_GEN_STR_H__
#define __YTIL_GEN_STR_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <alloca.h>
#include <sys/types.h>


typedef enum str_error
{
      E_STR_BINARY
    , E_STR_EMPTY
    , E_STR_INVALID_CSTR
    , E_STR_INVALID_DATA
    , E_STR_INVALID_FORMAT
    , E_STR_INVALID_LENGTH
    , E_STR_OUT_OF_BOUNDS
    , E_STR_UNREFERENCED
} str_eror_id;


typedef struct str       *str_ct;
typedef const struct str *str_const_ct;


// get length of str
size_t str_len(str_const_ct str);
// get capacity of str
size_t str_capacity(str_const_ct str);
// get allocated size of str
size_t str_memsize(str_const_ct str);
// get size of str head
size_t str_headsize(void);

// check if str is empty
bool str_is_empty(str_const_ct str);
// check if str head is transient
bool str_is_transient(str_const_ct str);
// check if str head is redirected (str_ref transient-heap str)
bool str_is_redirected(str_const_ct str);

// check if str data is allocated
bool str_data_is_heap(str_const_ct str);
// check if str data is const
bool str_data_is_const(str_const_ct str);
// check if str data is transient
bool str_data_is_transient(str_const_ct str);
// check if str data is binary
bool str_data_is_binary(str_const_ct str);

// readable raw getters

// get str data as const signed, fail if binary str
const char *str_c(str_const_ct str);
// get str data as const unsigned, fail if binary str
const unsigned char *str_uc(str_const_ct str);
// get str data as binary const void
const void *str_bc(str_const_ct str);
// get str data as binary const unsigned
const unsigned char *str_buc(str_const_ct str);

// writeable raw getters
// const strings are converted to heap strings, this may fail

// get str data as signed, fail if binary str
char *str_w(str_ct str);
// get str data as unsigned, fail if binary str
unsigned char *str_uw(str_ct str);
// get str data as binary void
void *str_bw(str_ct str);
// get str data as binary unsigned
unsigned char *str_buw(str_ct str);

// mark str for length update (if data was externally changed)
str_ct str_update(str_const_ct str);
// set str length (if data was externally changed)
str_ct str_set_len(str_const_ct str, size_t len);

// get first char of str
char str_first(str_const_ct str);
// get last char of str
char str_last(str_const_ct str);
// get char at str[pos]
char str_at(str_const_ct str, size_t pos);
// get unsigned char at str[pos]
unsigned char str_at_u(str_const_ct str, size_t pos);


// add reference, transient strings are converted to heap strings, this may fail
str_ct str_ref(str_const_ct str);
// remove reference, removal of last reference frees string
str_ct str_unref(str_const_ct str);
// return current reference count
size_t str_get_refs(str_const_ct str);


// set length to zero, keep capacity
str_ct str_clear(str_ct str);
// truncate str capacity to length
str_ct str_truncate(str_const_ct str);

// resize str, afterwards str is writeable
// convert const/transient data to heap if neccessary
str_ct str_resize(str_ct str, size_t len);
// resize str, afterwards str is writeable, initialize with c if grown
// convert const/transient data to heap if neccessary
str_ct str_resize_set(str_ct str, size_t len, char c);


// prepare new heap str of len
str_ct str_prepare(size_t len);
// prepare new heap str of len/capacity
str_ct str_prepare_c(size_t len, size_t cap);
// prepare new binary heap str of len
str_ct str_prepare_b(size_t len);
// prepare new binary heap str of len/capacity
str_ct str_prepare_bc(size_t len, size_t cap);

// prepare new heap str of len, initialize with c
str_ct str_prepare_set(size_t len, char c);
// prepare new heap str of len/capacity, initialize with c
str_ct str_prepare_set_c(size_t len, size_t cap, char c);
// prepare new binary heap str of len, initialize with c
str_ct str_prepare_set_b(size_t len, char c);
// prepare new binary heap str of len/capacity, initialize with c
str_ct str_prepare_set_bc(size_t len, size_t cap, char c);


// create new heap data str
str_ct  str_new_h(char *cstr);
// create new heap data str of len
str_ct  str_new_hn(char *cstr, size_t len);
// create new heap data str of len/capacity
str_ct  str_new_hnc(char *cstr, size_t len, size_t cap);
// create new const data str
str_ct  str_new_c(const char *cstr);
// create new const data str of len
str_ct  str_new_cn(const char *cstr, size_t len);
#define str_new_l(lit) str_new_cn(lit, sizeof(lit)-1)
// create new binary heap data str of len
str_ct  str_new_bh(void *data, size_t len);
// create new binary heap data str of len/capacity
str_ct  str_new_bhc(void *data, size_t len, size_t cap);
// create new binary const data str of len
str_ct  str_new_bc(const void *data, size_t len);


/* transient strings with heap data
 * - have one initial reference, unref before str head goes out of scope
 * - str_ref creates new heap str, transient str redirects to new heap str
 * - usage: encapsulate heap allocated result of a cstr function
 *
 * transient strings with const data
 * - have no initial reference, str_unref is an error
 * - str_ref creates new heap str, transient str will _not_ redirect to new str
 *   (because transient str owner holds no reference)
 * - immutable, const data will not be automatically converted to heap data
 * - usage: encapsulate string literals on the fly and dont worry
 *
 * transient strings with transient data
 * - like const data but mutable, also no autoconversion to heap data
 * - not resizeable larger than initial capacity
 * - usage: create formatted strings on the fly and dont worry
*/

// init transient-heap data str, with reference
str_ct  tstr_init_h(str_ct str, char *hstr);
#define tstr_new_h(str) tstr_init_h(alloca(str_headsize()), str)
// init transient-heap data str of len, with reference
str_ct  tstr_init_hn(str_ct str, char *hstr, size_t len);
#define tstr_new_hn(str, n) tstr_init_hn(alloca(str_headsize()), str, n)
// init transient-heap data str of len/capacity, with reference
str_ct  tstr_init_hnc(str_ct str, char *hstr, size_t len, size_t cap);
#define tstr_new_hnc(str, n, c) tstr_init_hnc(alloca(str_headsize()), str, n, c)
// init transient-const data str, without reference
str_ct  tstr_init_c(str_ct str, const char *cstr);
#define tstr_new_c(str) tstr_init_c(alloca(str_headsize()), str)
// init transient-const data str, without reference
str_ct  tstr_init_cn(str_ct str, const char *cstr, size_t len);
#define tstr_new_cn(str, n) tstr_init_cn(alloca(str_headsize()), str, n)
#define tstr_new_l(lit) tstr_init_cn(alloca(str_headsize()), lit, sizeof(lit)-1)
// init binary transient-heap data str of len, with reference
str_ct  tstr_init_bh(str_ct str, void *bin, size_t len);
#define tstr_new_bh(bin, n) tstr_init_bh(alloca(str_headsize()), bin, n)
// init binary transient-heap data str of len/capacity, with reference
str_ct  tstr_init_bhc(str_ct str, void *bin, size_t len, size_t cap);
#define tstr_new_bhc(bin, n, c) tstr_init_bhc(alloca(str_headsize()), bin, n, c)
// init binary transient-const data str, without reference
str_ct  tstr_init_bc(str_ct str, const void *bin, size_t len);
#define tstr_new_bc(bin, n) tstr_init_bc(alloca(str_headsize()), bin, n)
// init transient-transient data str of len, without reference
str_ct  tstr_init_tn(str_ct str, char *cstr, size_t len);
#define tstr_new_tn(str, n) tstr_init_tn(alloca(str_headsize()), str, n)
// init transient-transient data str of len/capacity, without reference
str_ct  tstr_init_tnc(str_ct str, char *cstr, size_t len, size_t cap);
#define tstr_new_tnc(str, n, c) tstr_init_tnc(alloca(str_headsize()), str, n, c)

#define STR(str)       tstr_new_c(str)
#define LIT(lit)       tstr_new_l(lit)
#define BIN(bin, size) tstr_new_bc(bin, size)


// create heap duplicate of str, const data is not duplicated
str_ct  str_dup(str_const_ct str);
// create heap duplicate of str with max len, const data is not duplicated
str_ct  str_dup_n(str_const_ct str, size_t len);
// create new heap str with copy of cstr
str_ct  str_dup_c(const char *cstr);
// create new heap str with copy of cstr with max len
str_ct  str_dup_cn(const char *cstr, size_t len);
#define str_dup_l(lit) str_dup_cn(lit, sizeof(lit)-1)
// create new heap str with copy of binary cstr
str_ct  str_dup_b(const void *data, size_t len);
// create new heap str from format
str_ct  str_dup_f(const char *fmt, ...) __attribute__((format(gnu_printf, 1, 2)));
// create new heap str from format with va_list
str_ct  str_dup_vf(const char *fmt, va_list ap) __attribute__((format(gnu_printf, 1, 0)));


// set str with heap data
str_ct  str_set_h(str_ct str, char *cstr);
// set str with heap data of len
str_ct  str_set_hn(str_ct str, char *cstr, size_t len);
// set str with heap data of len/capacity
str_ct  str_set_hnc(str_ct str, char *cstr, size_t len, size_t cap);
// set str with const data
str_ct  str_set_c(str_ct str, const char *cstr);
// set str with const data of len
str_ct  str_set_cn(str_ct str, const char *cstr, size_t len);
#define str_set_l(str, cstr) str_set_cn(str, cstr, sizeof(cstr)-1)
// set str with binary heap data of len
str_ct  str_set_bh(str_ct str, void *data, size_t len);
// set str with binary heap data of len/capacity
str_ct  str_set_bhc(str_ct str, void *data, size_t len, size_t cap);
// set str with binary const data
str_ct  str_set_bc(str_ct str, const void *data, size_t len);


// copy src str to dst str at pos, resize dst if necessary
str_ct  str_copy(str_ct dst, size_t pos, str_const_ct src);
// copy max n chars of src str to dst str at pos, resize dst if necessary
str_ct  str_copy_n(str_ct dst, size_t pos, str_const_ct src, size_t len);
// copy src cstr to dst str at pos, resize dst if necessary
str_ct  str_copy_c(str_ct dst, size_t pos, const char *src);
// copy max n chars of src cstr to dst str at pos, resize dst if necessary
str_ct  str_copy_cn(str_ct dst, size_t pos, const char *src, size_t len);
#define str_copy_l(dst, pos, lit) str_copy_cn(dst, pos, lit, sizeof(lit)-1)
// copy src data to dst str at pos, resize dst if necessary, dst is marked binary
str_ct  str_copy_b(str_ct dst, size_t pos, const void *src, size_t len);
// copy format cstr to dst str at pos, resize dst if necessary
str_ct  str_copy_f(str_ct dst, size_t pos, const char *fmt, ...) __attribute__((format(gnu_printf, 3, 4)));
// copy format cstr with va_list to dst str at pos, resize dst if necessary
str_ct  str_copy_vf(str_ct dst, size_t pos, const char *fmt, va_list ap) __attribute__((format(gnu_printf, 3, 0)));


// overwrite dst str at pos with src str
ssize_t str_overwrite(str_ct dst, size_t pos, str_const_ct src);
// overwrite max n chars in dst str at pos with src str
ssize_t str_overwrite_n(str_ct dst, size_t pos, str_const_ct src, size_t len);
// overwrite dst str at pos with src cstr
ssize_t str_overwrite_c(str_ct dst, size_t pos, const char *src);
// overwrite max n chars in dst str at pos with src cstr
ssize_t str_overwrite_cn(str_ct dst, size_t pos, const char *src, size_t len);
#define str_overwrite_l(dst, pos, lit) str_overwrite_cn(dst, pos, lit, sizeof(lit)-1)
// overwrite dst str at pos with src data, dst is marked binary
ssize_t str_overwrite_b(str_ct dst, size_t pos, const void *src, size_t len);
// overwrite dst str at pos with format cstr
ssize_t str_overwrite_f(str_ct dst, size_t pos, const char *fmt, ...) __attribute__((format(gnu_printf, 3, 4)));
// overwrite max n chars in dst str at pos with format cstr
ssize_t str_overwrite_fn(str_ct dst, size_t pos, size_t len, const char *fmt, ...) __attribute__((format(gnu_printf, 4, 5)));
// overwrite dst str at pos with format cstr with va_list
ssize_t str_overwrite_vf(str_ct dst, size_t pos, const char *fmt, va_list ap) __attribute__((format(gnu_printf, 3, 0)));
// overwrite max n chars in dst str at pos with format cstr with va_list
ssize_t str_overwrite_vfn(str_ct dst, size_t pos, size_t len, const char *fmt, va_list ap) __attribute__((format(gnu_printf, 4, 0)));


// prepend prefix str to str
str_ct  str_prepend(str_ct str, str_const_ct prefix);
// prepend max n chars of prefix str to str
str_ct  str_prepend_n(str_ct str, str_const_ct prefix, size_t len);
// prepend prefix cstr to str
str_ct  str_prepend_c(str_ct str, const char *prefix);
// prepend max n chars of prefix cstr to str
str_ct  str_prepend_cn(str_ct str, const char *prefix, size_t len);
#define str_prepend_l(str, prefix) str_prepend_cn(str, prefix, sizeof(prefix)-1)
// prepend prefix data to str, mark str binary
str_ct  str_prepend_b(str_ct str, const void *prefix, size_t len);
// prepend n chars to str, initialize with c
str_ct  str_prepend_set(str_ct str, size_t len, char c);
// prepend format cstr to str
str_ct  str_prepend_f(str_ct str, const char *fmt, ...) __attribute__((format(gnu_printf, 2, 3)));
// prepend format cstr with va_list to str
str_ct  str_prepend_vf(str_ct str, const char *fmt, va_list ap) __attribute__((format(gnu_printf, 2, 0)));

// append suffix str to str
str_ct  str_append(str_ct str, str_const_ct suffix);
// append max n chars of suffix str to str
str_ct  str_append_n(str_ct str, str_const_ct suffix, size_t len);
// append suffix cstr to str
str_ct  str_append_c(str_ct str, const char *suffix);
// append max n chars of suffix cstr to str
str_ct  str_append_cn(str_ct str, const char *suffix, size_t len);
#define str_append_l(str, suffix) str_append_cn(str, suffix, sizeof(suffix)-1)
// append suffix data to str, mark str binary
str_ct  str_append_b(str_ct str, const void *suffix, size_t len);
// append n chars to str, initialize with c
str_ct  str_append_set(str_ct str, size_t len, char c);
// append format cstr to str
str_ct  str_append_f(str_ct str, const char *fmt, ...) __attribute__((format(gnu_printf, 2, 3)));
// append format cstr with va_list to str
str_ct  str_append_vf(str_ct str, const char *fmt, va_list ap) __attribute__((format(gnu_printf, 2, 0)));

// insert sub str into str at pos
str_ct  str_insert(str_ct str, size_t pos, str_const_ct sub);
// insert max n chars of sub str into str at pos
str_ct  str_insert_n(str_ct str, size_t pos, str_const_ct sub, size_t len);
// insert sub cstr into str at pos
str_ct  str_insert_c(str_ct str, size_t pos, const char *sub);
// insert max n chars of sub cstr into str at pos
str_ct  str_insert_cn(str_ct str, size_t pos, const char *sub, size_t len);
#define str_insert_l(str, pos, sub) str_insert_cn(str, pos, sub, sizeof(sub)-1)
// insert sub data into str at pos, mark str binary
str_ct  str_insert_b(str_ct str, size_t pos, const void *sub, size_t len);
// insert n chars into str at pos, initialize with c
str_ct  str_insert_set(str_ct str, size_t pos, size_t len, char c);
// insert format cstr into str at pos
str_ct  str_insert_f(str_ct str, size_t pos, const char *fmt, ...) __attribute__((format(gnu_printf, 3, 4)));
// insert format cstr with va_list into str at pos
str_ct  str_insert_vf(str_ct str, size_t pos, const char *fmt, va_list ap) __attribute__((format(gnu_printf, 3, 0)));


#endif