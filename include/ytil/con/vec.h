/*
 * Copyright (c) 2012-2019 Martin Rödel aka Yomin
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

#ifndef YTIL_CON_VEC_H_INCLUDED
#define YTIL_CON_VEC_H_INCLUDED

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/types.h>


typedef enum vec_error
{
      E_VEC_CALLBACK
    , E_VEC_EMPTY
    , E_VEC_INVALID_ELEMSIZE
    , E_VEC_NO_BUFFER
    , E_VEC_NOT_FOUND
    , E_VEC_NULL_REQUEST
    , E_VEC_OUT_OF_BOUNDS
    , E_VEC_UNALIGNED
} vec_error_id;


struct vector;

typedef       struct vector *vec_ct;
typedef const struct vector *vec_const_ct;

// return size of vector elem
typedef size_t (*vec_size_cb)(vec_const_ct vec, const void *elem, void *ctx);
// free vector elem
typedef void   (*vec_dtor_cb)(vec_const_ct vec, void *elem, void *ctx);
// return true if pred matches elem
typedef bool   (*vec_pred_cb)(vec_const_ct vec, const void *elem, void *ctx);
// return 0 to continue fold, anything else stops fold
typedef int    (*vec_fold_cb)(vec_const_ct vec, size_t index, void *elem, void *ctx);
// duplicate src vector elem into dst, return 0 on success
typedef int    (*vec_clone_cb)(vec_const_ct src_vec, void *dst, const void *src, void *ctx);


// create new vector
vec_ct vec_new(size_t capacity, size_t elemsize);
// free vector
void   vec_free(vec_ct vec);
// free vector, apply dtor on each elem
void   vec_free_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx);
// free vector if empty
vec_ct vec_free_if_empty(vec_ct vec);

// remove all elems from vector
void vec_clear(vec_ct vec);
// remove all elems from vector, apply dtor on each elem
void vec_clear_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx);

// duplicate vector
vec_ct vec_clone(vec_const_ct vec);
// duplicate vector, duplicate elem with clone function
vec_ct vec_clone_f(vec_const_ct vec, vec_clone_cb clone, vec_dtor_cb dtor, const void *ctx);

// check if vector is empty
bool   vec_is_empty(vec_const_ct vec);
// get vector elem count
size_t vec_size(vec_const_ct vec);
// get vector elem size
size_t vec_elemsize(vec_const_ct vec);
// get vector capacity
size_t vec_capacity(vec_const_ct vec);
// get vector min capacity
size_t vec_min_capacity(vec_const_ct vec);
// get allocated size of vector
size_t vec_memsize(vec_const_ct vec);
// get allocated size of vector, apply sfun on each elem to determine size
size_t vec_memsize_f(vec_const_ct vec, vec_size_cb sfun, const void *ctx);

// return pointer to vector elem at pos, negative pos counts from last elem
void *vec_at(vec_const_ct vec, ssize_t pos);
// return vector elem at pos as pointer, negative pos counts from last elem
void *vec_at_p(vec_const_ct vec, ssize_t pos);
// return pointer to first vector elem
void *vec_first(vec_const_ct vec);
// return first vector elem as pointer
void *vec_first_p(vec_const_ct vec);
// return pointer to last vector elem
void *vec_last(vec_const_ct vec);
// return last vector elem as pointer
void *vec_last_p(vec_const_ct vec);

// get position of elem in vector
ssize_t vec_pos(vec_const_ct vec, const void *elem);

// copy vector elem at pos into dst, negative pos counts from last elem
int     vec_get(vec_const_ct vec, void *dst, ssize_t pos);
// copy n vector elems from pos till pos+n-1 into dst
// negative pos counts from last elem, return count of elems copied
ssize_t vec_get_n(vec_const_ct vec, void *dst, ssize_t pos, size_t n);
// copy first vector elem into dst
int     vec_get_first(vec_const_ct vec, void *dst);
// copy last vector elem into dst
int     vec_get_last(vec_const_ct vec, void *dst);

// append empty elem to vector
void *vec_push(vec_ct vec);
// append n empty elems to vector
void *vec_push_n(vec_ct vec, size_t n);
// append elem to vector
void *vec_push_e(vec_ct vec, const void *elem);
// append elem as pointer to vector
void *vec_push_p(vec_ct vec, const void *ptr);
// append n elems to vector
void *vec_push_en(vec_ct vec, size_t n, const void *elems);
// append n elems to vector with arg list
void *vec_push_args(vec_ct vec, size_t n, ...);
// append n pointer to vector with arg list
void *vec_push_args_p(vec_ct vec, size_t n, ...);
// append n elems to vector with arg list, va_list version
void *vec_push_args_v(vec_ct vec, size_t n, va_list ap);
// append n pointer to vector with arg list, va_list version
void *vec_push_args_pv(vec_ct vec, size_t n, va_list ap);

// insert empty elem at pos into vector, negative pos counts from last elem
void *vec_insert(vec_ct vec, ssize_t pos);
// insert elem at pos into vector, negative pos counts from last elem
void *vec_insert_e(vec_ct vec, ssize_t pos, const void *elem);
// insert elem as pointer at pos into vector, negative pos counts from last elem
void *vec_insert_p(vec_ct vec, ssize_t pos, const void *ptr);
// insert n empty elems at pos into vector, negative pos counts from last elem
void *vec_insert_n(vec_ct vec, ssize_t pos, size_t n);
// insert n elems at pos into vector, negative pos counts from last elem
void *vec_insert_en(vec_ct vec, ssize_t pos, size_t n, const void *elems);

// insert empty elem before dst into vector
void *vec_insert_before(vec_ct vec, const void *dst);
// insert elem before dst into vector
void *vec_insert_before_e(vec_ct vec, const void *dst, const void *elem);
// insert elem as pointer before dst into vector
void *vec_insert_before_p(vec_ct vec, const void *dst, const void *ptr);
// insert n empty elems before dst into vector
void *vec_insert_before_n(vec_ct vec, const void *dst, size_t n);
// insert n elems before dst into vector
void *vec_insert_before_en(vec_ct vec, const void *dst, size_t n, const void *elems);

// insert empty elem after dst into vector
void *vec_insert_after(vec_ct vec, const void *dst);
// insert elem after dst into vector
void *vec_insert_after_e(vec_ct vec, const void *dst, const void *elem);
// insert elem as pointer after dst into vector
void *vec_insert_after_p(vec_ct vec, const void *dst, const void *ptr);
// insert n empty elems after dst into vector
void *vec_insert_after_n(vec_ct vec, const void *dst, size_t n);
// insert n elems after dst into vector
void *vec_insert_after_en(vec_ct vec, const void *dst, size_t n, const void *elems);

// remove last elem from vector
int     vec_pop(vec_ct vec);
// remove last n elems from vector, return count of elems removed
ssize_t vec_pop_n(vec_ct vec, size_t n);
// copy last elem from vector into dst and remove it from vector
int     vec_pop_e(vec_ct vec, void *dst);
// return last elem from vector as pointer and remove it from vector
void   *vec_pop_p(vec_ct vec);
// copy last n elems from vector into dst and remove them from vector
ssize_t vec_pop_en(vec_ct vec, void *dst, size_t n);
// apply dtor to last elem from vector and remove it from vector
int     vec_pop_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx);
// apply dtor to last n elems from vector and remove them from vector
ssize_t vec_pop_fn(vec_ct vec, size_t n, vec_dtor_cb dtor, const void *ctx);

// remove elem from vector
int     vec_remove(vec_ct vec, void *elem);
// remove n elems from vector, starting at elem, return count of elems removed
ssize_t vec_remove_n(vec_ct vec, void *elem, size_t n);
// remove elem at pos from vector, negative pos counts from last elem
int     vec_remove_at(vec_ct vec, ssize_t pos);
// remove n elems from pos till pos+n-1 from vector
// negative pos counts from last elem, return count of elems removed
ssize_t vec_remove_at_n(vec_ct vec, ssize_t pos, size_t n);
// copy elem at pos from vector into dst and remove it from vector
// negative pos counts from last elem
int     vec_remove_at_e(vec_ct vec, void *dst, ssize_t pos);
// return elem at pos from vector as pointer and remove it from vector
// negative pos counts from last elem
void   *vec_remove_at_p(vec_ct vec, ssize_t pos);
// copy n elems from pos till pos+n-1 from vector into dst and remove them from vector
// negative pos counts from last elem, return count of elems removed
ssize_t vec_remove_at_en(vec_ct vec, void *dst, ssize_t pos, size_t n);
// apply dtor to elem at pos from vector and remove it from vector
// negative pos counts from last elem
int     vec_remove_at_f(vec_ct vec, ssize_t pos, vec_dtor_cb dtor, const void *ctx);
// apply dtor to elems from pos till pos+n-1 from vector and remove them from vector
// negative pos counts from last elem, return count of elems removed
ssize_t vec_remove_at_fn(vec_ct vec, ssize_t pos, size_t n, vec_dtor_cb dtor, const void *ctx);

// find first elem in vector by pred
void   *vec_find(vec_const_ct vec, vec_pred_cb pred, const void *ctx);
// find first elem in vector by pred, return as pointer
void   *vec_find_p(vec_const_ct vec, vec_pred_cb pred, const void *ctx);
// find last elem in vector by pred
void   *vec_find_r(vec_const_ct vec, vec_pred_cb pred, const void *ctx);
// find last elem in vector by pred, return as pointer
void   *vec_find_rp(vec_const_ct vec, vec_pred_cb pred, const void *ctx);
// find first elem pos in vector by pred
ssize_t vec_find_pos(vec_const_ct vec, vec_pred_cb pred, const void *ctx);
// find last elem pos in vector by pred
ssize_t vec_find_pos_r(vec_const_ct vec, vec_pred_cb pred, const void *ctx);
// find first elem in vector by pred and copy it to dst
int     vec_find_get(vec_const_ct vec, void *dst, vec_pred_cb pred, const void *ctx);
// find last elem in vector by pred and copy it to dst
int     vec_find_get_r(vec_const_ct vec, void *dst, vec_pred_cb pred, const void *ctx);
// find first elem in vector by pred and remove it
int     vec_find_remove(vec_ct vec, vec_pred_cb pred, const void *ctx);
// find first elem in vector by pred, remove it and return as pointer
void   *vec_find_remove_p(vec_ct vec, vec_pred_cb pred, const void *ctx);
// find first elem in vector by pred, apply dtor and remove it
int     vec_find_remove_f(vec_ct vec, vec_pred_cb pred, const void *pred_ctx, vec_dtor_cb dtor, const void *dtor_ctx);
// find last elem in vector by pred and remove it
int     vec_find_remove_r(vec_ct vec, vec_pred_cb pred, const void *ctx);
// find last elem in vector by pred, remove it and return as pointer
void   *vec_find_remove_rp(vec_ct vec, vec_pred_cb pred, const void *ctx);
// find last elem in vector by pred, apply dtor and remove it
int     vec_find_remove_rf(vec_ct vec, vec_pred_cb pred, const void *pred_ctx, vec_dtor_cb dtor, const void *dtor_ctx);
// find all elems in vector by pred and remove them
size_t  vec_find_remove_all(vec_ct vec, vec_pred_cb pred, const void *ctx);
// find all elems in vector by pred, apply dtor and remove them
size_t  vec_find_remove_all_f(vec_ct vec, vec_pred_cb pred, const void *pred_ctx, vec_dtor_cb dtor, const void *dtor_ctx);

// swap elem at pos1 and pos2 in vector, negative pos counts from last elem
int vec_swap(vec_ct vec, ssize_t pos1, ssize_t pos2);

// get vector buffer/size/capacity, reset vector buffer to NULL
int vec_get_buffer(vec_ct vec, void **buf, size_t *size, size_t *capacity);

// truncate vector to n elems
size_t vec_truncate(vec_ct vec, size_t size);
// truncate vector to n elems, apply dtor to removed elems
size_t vec_truncate_f(vec_ct vec, size_t size, vec_dtor_cb dtor, const void *ctx);

// set vector capacity to shrink/grow vector buffer
int vec_set_capacity(vec_ct vec, size_t capacity);
// set vector capacity to shrink/grow vector buffer, apply dtor to removed elems
int vec_set_capacity_f(vec_ct vec, size_t capacity, vec_dtor_cb dtor, const void *ctx);

// apply fold to every vector elem, begin with first elem
int vec_fold(vec_const_ct vec, vec_fold_cb fold, const void *ctx);
// apply fold to every vector elem, begin with last elem
int vec_fold_r(vec_const_ct vec, vec_fold_cb fold, const void *ctx);

#endif
