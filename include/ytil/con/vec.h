/*
 * Copyright (c) 2012-2020 Martin Rödel aka Yomin
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

/// \file

#ifndef YTIL_CON_VEC_H_INCLUDED
#define YTIL_CON_VEC_H_INCLUDED

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/types.h>
#include <ytil/gen/error.h>


/// vector error
typedef enum vec_error
{
    E_VEC_CALLBACK,         ///< callback error
    E_VEC_EMPTY,            ///< vector is empty
    E_VEC_NO_BUFFER,        ///< no buffer allocated
    E_VEC_NOT_FOUND,        ///< element not found
    E_VEC_OUT_OF_BOUNDS,    ///< out of bounds element access
} vec_error_id;

/// vector error type declaration
ERROR_DECLARE(VEC);


struct vector;
typedef       struct vector *vec_ct;        ///< vector type
typedef const struct vector *vec_const_ct;  ///< const vector type

/// vector element size callback
///
/// \param vec      vector
/// \param elem     element to get size of
/// \param ctx      callback context
///
/// \returns        size of element
typedef size_t (*vec_size_cb)(vec_const_ct vec, const void *elem, void *ctx);

/// vector element dtor callback
///
/// \param vec      vector
/// \param elem     element to destroy
/// \param ctx      callback context
typedef void (*vec_dtor_cb)(vec_const_ct vec, void *elem, void *ctx);

/// vector element predicate callback
///
/// \param vec      vector
/// \param elem     element to check
/// \param ctx      callback context
///
/// \retval true    \p elem matches predicate
/// \retval false   \p elem does not match predicate
typedef bool (*vec_pred_cb)(vec_const_ct vec, const void *elem, void *ctx);

/// vector element fold callback
///
/// \param vec      vector
/// \param index    index of element in vector
/// \param elem     element
/// \param ctx      callback context
///
/// \retval 0       continue fold
/// \retval <0      stop fold with error
/// \retval >0      stop fold
typedef int (*vec_fold_cb)(vec_const_ct vec, size_t index, void *elem, void *ctx);

/// vector element clone callback
///
/// \param      src_vec     source vector
/// \param[out] dst         element to fill
/// \param      elem        element to copy
/// \param      ctx         callback context
///
/// \retval 0               success
/// \retval !=0             error
typedef int (*vec_clone_cb)(vec_const_ct src_vec, void *dst, const void *src, void *ctx);

/// vector element sort callback
///
/// \param elem1    first element to compare
/// \param elem2    second elemet to compare
/// \param ctx      callback context
///
/// \retval <0      \p elem1 is less than \p elem2
/// \retval  0      \p elem1 matches \p elem2
/// \retval >0      \p elem1 is greater than \p elem2
typedef int (*vec_sort_cb)(const void *elem1, const void *elem2, void *ctx);


/// Create new vector of default capacity.
///
/// \param elemsize     element size
///
/// \returns                    new vector
/// \retval NULL/E_GENERIC_OOM  out of memory
vec_ct vec_new(size_t elemsize);

/// Create new vector.
///
/// \param capacity     initial vector capacity in number of elements
/// \param elemsize     element size
///
/// \returns                    new vector
/// \retval NULL/E_GENERIC_OOM  out of memory
vec_ct vec_new_c(size_t capacity, size_t elemsize);

/// Free vector.
///
/// \param vec      vector
void vec_free(vec_ct vec);

/// Destroy elements and free vector.
///
///
/// \param vec      vector
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
void vec_free_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx);

/// Free vector if empty.
///
/// \param vec      vector
///
/// \retval vec     vector is not empty
/// \retval NULL    vector was empty and was freed
vec_ct vec_free_if_empty(vec_ct vec);

/// Destroy elements and free vector if empty.
///
/// \param vec      vector
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval vec     vector is not empty
/// \retval NULL    vector was empty and was freed
vec_ct vec_free_if_empty_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx);

/// Remove all elements from vector.
///
/// \param vec      vector
void vec_clear(vec_ct vec);

/// Destroy and remove all elements from vector.
///
/// \param vec      vector
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
void vec_clear_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx);

/// Clone vector.
///
/// \param vec      vector
///
/// \returns                        new vector
/// \retval NULL/E_GENERIC_OOM      out of memory
vec_ct vec_clone(vec_const_ct vec);

/// Clone vector with clone callback.
///
/// If clone callback is given, invoke clone on each element,
/// else memdup each element.
/// If cloning fails and dtor callback is given,
/// invoke dtor on each already cloned element.
///
/// \param vec      vector
/// \param clone    callback to duplicate element, may be NULL
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p clone and \p dtor context
///
/// \returns                        new vector
/// \retval NULL/E_GENERIC_OOM      out of memory
/// \retval NULL/E_VEC_CALLBACK     \p clone callback error
vec_ct vec_clone_f(vec_const_ct vec, vec_clone_cb clone, vec_dtor_cb dtor, const void *ctx);

/// Check if vector is empty.
///
/// \param vec      vector
///
/// \retval true    vector is empty
/// \retval false   vector is not empty
bool vec_is_empty(vec_const_ct vec);

/// Get number of vector elements.
///
/// \param vec      vector
///
/// \returns        number of vector elements
size_t vec_size(vec_const_ct vec);

/// Get vector element size.
///
/// \param vec      vector
///
/// \returns        element size
size_t vec_elemsize(vec_const_ct vec);

/// Get vector capacity.
///
/// \param vec      vector
///
/// \returns        capacity in number of elements
size_t vec_capacity(vec_const_ct vec);

/// Get vector minimum capacity.
///
/// \param vec      vector
///
/// \returns        minimum capacity in number of elements
size_t vec_min_capacity(vec_const_ct vec);

/// Get allocated size of vector.
///
/// \param vec      vector
///
/// \returns        allocated size in bytes
size_t vec_memsize(vec_const_ct vec);

/// Get allocated size of vector and elements.
///
/// The returned size of \p size callback is added to the element size.
///
/// \param vec      vector
/// \param size     callback to determine size of element, may be NULL
/// \param ctx      \p size context
///
/// \returns        allocated size in bytes
size_t vec_memsize_f(vec_const_ct vec, vec_size_cb size, const void *ctx);

/// Get pointer to vector element at position.
///
/// \param vec      vector
/// \param pos      position of element, negative value counts from last element
///
/// \returns                            pointer to element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p pos is out of bounds
void *vec_at(vec_const_ct vec, ssize_t pos);

/// Get vector element casted to pointer at position.
///
/// \note Vector element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param vec      vector
/// \param pos      position of element, negative value counts from last element
///
/// \returns                            element casted to pointer
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p pos is out of bounds
void *vec_at_p(vec_const_ct vec, ssize_t pos);

/// Get pointer to first vector element.
///
/// \param vec      vector
///
/// \returns                    pointer to element
/// \retval NULL/E_VEC_EMPTY    vector is empty
void *vec_first(vec_const_ct vec);

/// Get first vector element casted to pointer.
///
/// \note Vector element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param vec      vector
///
/// \returns                    element casted to pointer
/// \retval NULL/E_VEC_EMPTY    vector is empty
void *vec_first_p(vec_const_ct vec);

/// Get pointer to last vector element.
///
/// \param vec      vector
/// \returns                    pointer to element
/// \retval NULL/E_VEC_EMPTY    vector is empty
void *vec_last(vec_const_ct vec);

/// Get last vector element casted to pointer.
///
/// \note Vector element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param vec      vector
///
/// \returns                    element casted to pointer
/// \retval NULL/E_VEC_EMPTY    vector is empty
void *vec_last_p(vec_const_ct vec);

/// Get position of element in vector.
///
/// \param vec      vector
/// \param elem     element to get position of
///
/// \returns                        element position
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p elem not within vector memory
ssize_t vec_pos(vec_const_ct vec, const void *elem);

/// Get element from vector.
///
/// \param      vec     vector
/// \param[out] dst     element to fill, may be NULL
/// \param      pos     position of element, negative value counts from last element
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos is out of bounds
int vec_get(vec_const_ct vec, void *dst, ssize_t pos);

/// Get n elements from vector.
///
/// \param      vec     vector
/// \param[out] dst     elements to fill, may be NULL
/// \param      pos     position of first element, negative value counts from last element
/// \param      n       number of elements to get
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos or \p pos + \p n is out of bounds
int vec_get_n(vec_const_ct vec, void *dst, ssize_t pos, size_t n);

/// Get first element from vector.
///
/// \param      vec     vector
/// \param[out] dst     element to fill, may be NULL
///
/// \retval 0               success
/// \retval -1/E_VEC_EMPTY  vector is empty
int vec_get_first(vec_const_ct vec, void *dst);

/// Get last element from vector.
///
/// \param      vec     vector
/// \param[out] dst     element to fill, may be NULL
///
/// \retval 0               success
/// \retval -1/E_VEC_EMPTY  vector is empty
int vec_get_last(vec_const_ct vec, void *dst);

/// Push empty element onto vector.
///
/// \param vec      vector
///
/// \returns                    pointer to new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *vec_push(vec_ct vec);

/// Push n empty elements onto vector.
///
/// \param vec      vector
/// \param n        number of elements to push
///
/// \returns                    pointer to first new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *vec_push_n(vec_ct vec, size_t n);

/// Push element onto vector.
///
/// \param vec      vector
/// \param elem     pointer to element, may be NULL
///
/// \returns                    pointer to new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *vec_push_e(vec_ct vec, const void *elem);

/// Push pointer onto vector.
///
/// \note Vector element size must be equal to sizeof(void*).
///
/// \param vec      vector
/// \param ptr      pointer
///
/// \returns                    pointer to new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *vec_push_p(vec_ct vec, const void *ptr);

/// Push n elements onto vector.
///
/// \param vec      vector
/// \param n        number of elements to push
/// \param elems    pointer to array of \p n elements, may be NULL
///
/// \returns                    pointer to first new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *vec_push_en(vec_ct vec, size_t n, const void *elems);

/// Push n elements with variadic list onto vector.
///
/// \param vec      vector
/// \param n        number of elements to push
/// \param ...      variadic list of elements
///
/// \returns                    pointer to first new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *vec_push_a(vec_ct vec, size_t n, ...);

/// Push n pointers with variadic list onto vector.
///
/// \note Vector element size must be equal to sizeof(void*).
///
/// \param vec      vector
/// \param n        number of pointers to push
/// \param ...      variadic list of pointers
///
/// \returns                    pointer to first new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *vec_push_ap(vec_ct vec, size_t n, ...);

/// Push n elements with variadic list pointer onto vector.
///
/// \param vec      vector
/// \param n        number of elements to push
/// \param ap       variadic list pointer
///
/// \returns                    pointer to first new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *vec_push_av(vec_ct vec, size_t n, va_list ap);

/// Push n pointers with variadic list pointer onto vector.
///
/// \note Vector element size must be equal to sizeof(void*).
///
/// \param vec      vector
/// \param n        number of pointers to push
/// \param ap       variadic list pointer
///
/// \returns                    pointer to first new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *vec_push_apv(vec_ct vec, size_t n, va_list ap);

/// Insert empty element into vector.
///
/// \param vec      vector
/// \param pos      insert position, negative value counts from last element
///
/// \returns                            pointer to new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p pos is out of bounds
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert(vec_ct vec, ssize_t pos);

/// Insert empty element into vector.
///
/// \param vec      vector
/// \param pos      insert position, negative value counts from last element
/// \param elem     pointer to element to insert, may be NULL
///
/// \returns                            pointer to new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p pos is out of bounds
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_e(vec_ct vec, ssize_t pos, const void *elem);

/// Insert pointer into vector.
///
/// \note Vector element size must be equal to sizeof(void*).
///
/// \param vec      vector
/// \param pos      insert position, negative value counts from last element
/// \param ptr      pointer to insert
///
/// \returns                            pointer to new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p pos is out of bounds
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_p(vec_ct vec, ssize_t pos, const void *ptr);

/// Insert n empty elements into vector.
///
/// \param vec      vector
/// \param pos      insert position, negative value counts from last element
/// \param n        number of elements to insert
///
/// \returns                            pointer to first new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p pos is out of bounds
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_n(vec_ct vec, ssize_t pos, size_t n);

/// Insert n elements into vector.
///
/// \param vec      vector
/// \param pos      insert position, negative value counts from last element
/// \param n        number of elements to insert
/// \param elems    pointer to array of \p n elements to insert, may be NULL
///
/// \returns                            pointer to first new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p pos is out of bounds
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_en(vec_ct vec, ssize_t pos, size_t n, const void *elems);

/// Insert empty element before other element into vector.
///
/// \param vec      vector
/// \param dst      pointer to element to insert before
///
/// \returns                            pointer to new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p dst is not within vector memory
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_before(vec_ct vec, const void *dst);

/// Insert element before other element into vector.
///
/// \param vec      vector
/// \param dst      pointer to element to insert before
/// \param elem     pointer to element to insert, may be NULL
///
/// \returns                            pointer to new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p dst is not within vector memory
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_before_e(vec_ct vec, const void *dst, const void *elem);

/// Insert pointer before other element into vector.
///
/// \note Vector element size must be equal to sizeof(void*).
///
/// \param vec      vector
/// \param dst      pointer to element to insert before
/// \param ptr      pointer to insert
///
/// \returns                            pointer to new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p dst is not within vector memory
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_before_p(vec_ct vec, const void *dst, const void *ptr);

/// Insert n empty elements before other element into vector.
///
/// \param vec      vector
/// \param dst      pointer to element to insert before
/// \param n        number of elements to insert
///
/// \returns                            pointer to first new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p dst is not within vector memory
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_before_n(vec_ct vec, const void *dst, size_t n);

/// Insert n elements before other element into vector.
///
/// \param vec      vector
/// \param dst      pointer to element to insert before
/// \param n        number of elements to insert
/// \param elems    pointer to array of \p n elements to insert, may be NULL
///
/// \returns                            pointer to first new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p dst is not within vector memory
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_before_en(vec_ct vec, const void *dst, size_t n, const void *elems);

/// Insert empty element after other element into vector.
///
/// \param vec      vector
/// \param dst      pointer to element to insert after
///
/// \returns                            pointer to new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p dst is not within vector memory
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_after(vec_ct vec, const void *dst);

/// Insert element after other element into vector.
///
/// \param vec      vector
/// \param dst      pointer to element to insert after
/// \param elem     pointer to element to insert, may be NULL
///
/// \returns                            pointer to new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p dst is not within vector memory
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_after_e(vec_ct vec, const void *dst, const void *elem);

/// Insert pointer after other element into vector.
///
/// \note Vector element size must be equal to sizeof(void*).
///
/// \param vec      vector
/// \param dst      pointer to element to insert after
/// \param ptr      pointer to insert
///
/// \returns                            pointer to new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p dst is not within vector memory
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_after_p(vec_ct vec, const void *dst, const void *ptr);

/// Insert n empty elements after other element into vector.
///
/// \param vec      vector
/// \param dst      pointer to element to insert after
/// \param n        number of elements to insert
///
/// \returns                            pointer to first new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p dst is not within vector memory
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_after_n(vec_ct vec, const void *dst, size_t n);

/// Insert n elements after other element into vector.
///
/// \param vec      vector
/// \param dst      pointer to element to insert after
/// \param n        number of elements to insert
/// \param elems    pointer to array of \p n elements to insert, may be NULL
///
/// \returns                            pointer to first new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p dst is not within vector memory
/// \retval NULL/E_GENERIC_OOM          out of memory
void *vec_insert_after_en(vec_ct vec, const void *dst, size_t n, const void *elems);

/// Set vector element at position.
///
/// \param vec      vector
/// \param pos      position of element, negative value counts from last element
/// \param elem     pointer to element to copy, may be NULL
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos is out of bounds
int vec_set(vec_ct vec, ssize_t pos, const void *elem);

/// Destroy and set vector element at position.
///
/// \param vec      vector
/// \param pos      position of element, negative value counts from last element
/// \param elem     pointer to element to copy, may be NULL
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos is out of bounds
int vec_set_f(vec_ct vec, ssize_t pos, const void *elem, vec_dtor_cb dtor, const void *ctx);

/// Remove last element.
///
/// \param vec      vector
///
/// \retval 0               success
/// \retval -1/E_VEC_EMPTY  vector is empty
int vec_pop(vec_ct vec);

/// Get and remove last element.
///
/// \param      vec     vector
/// \param[out] dst     pointer to element to fill, may be NULL
///
/// \retval 0               success
/// \retval -1/E_VEC_EMPTY  vector is empty
int vec_pop_e(vec_ct vec, void *dst);

/// Remove and return last element casted to pointer.
///
/// \note Vector element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param vec      vector
///
/// \returns                    element casted to pointer
/// \retval NULL/E_VEC_EMPTY    vector is empty
void *vec_pop_p(vec_ct vec);

/// Destroy and remove last element.
///
/// \param vec      vector
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0               success
/// \retval -1/E_VEC_EMPTY  vector is empty
int vec_pop_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx);

/// Remove last n elements.
///
/// \param vec      vector
/// \param n        number of elements to remove
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  not enough elements to remove
int vec_pop_n(vec_ct vec, size_t n);

/// Get and remove last n elements.
///
/// \param      vec     vector
/// \param[out] dst     pointer to element array to fill, may be NULL
/// \param      n       number of elements to remove
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  not enough elements to remove
int vec_pop_en(vec_ct vec, void *dst, size_t n);

/// Destroy and remove last n elements.
///
/// \param vec      vector
/// \param n        number of elements to remove
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  not enough elements to remove
int vec_pop_fn(vec_ct vec, size_t n, vec_dtor_cb dtor, const void *ctx);

/// Remove element.
///
/// \param vec      vector
/// \param elem     element to remove
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p elem is not within vector memory
int vec_remove(vec_ct vec, void *elem);

/// Destroy and remove element.
///
/// \param vec      vector
/// \param elem     element to remove
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p elem is not within vector memory
int vec_remove_f(vec_ct vec, void *elem, vec_dtor_cb dtor, const void *ctx);

/// Remove n elements.
///
/// \param vec      vector
/// \param elem     first element to remove
/// \param n        number of elements to remove
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  not all elements are within vector memory
int vec_remove_n(vec_ct vec, void *elem, size_t n);

/// Destroy and remove n elements.
///
/// \param vec      vector
/// \param elem     first element to remove
/// \param n        number of elements to remove
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  not all elements are within vector memory
int vec_remove_fn(vec_ct vec, void *elem, size_t n, vec_dtor_cb dtor, const void *ctx);

/// Remove element at position.
///
/// \param vec      vector
/// \param pos      position of element, negative value counts from last element
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos is out of bounds
int vec_remove_at(vec_ct vec, ssize_t pos);

/// Get and remove element at position.
///
/// \param      vec     vector
/// \param[out] dst     pointer to element to fill, may be NULL
/// \param      pos     position of element, negative value counts from last element
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos is out of bounds
int vec_remove_at_e(vec_ct vec, void *dst, ssize_t pos);

/// Remove and return element casted to pointer at position.
///
/// \note Vector element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param vec      vector
/// \param pos      position of element, negative value counts from last element
///
/// \returns                            element casted to pointer
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p pos is out of bounds
void *vec_remove_at_p(vec_ct vec, ssize_t pos);

/// Destroy and remove element at position.
///
/// \param vec      vector
/// \param pos      position of element, negative value counts from last element
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos is out of bounds
int vec_remove_at_f(vec_ct vec, ssize_t pos, vec_dtor_cb dtor, const void *ctx);

/// Remove n elements at position.
///
/// \param vec      vector
/// \param pos      position of first element, negative value counts from last element
/// \param n        number of elements to remove
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos or \p pos + \p n is out of bounds
int vec_remove_at_n(vec_ct vec, ssize_t pos, size_t n);

/// Get and remove n elements at position.
///
/// \param      vec     vector
/// \param[out] dst     pointer to element array to fill, may be NULL
/// \param      pos     position of first element, negative value counts from last element
/// \param      n       number of elements to remove
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos or \p pos + \p n is out of bounds
int vec_remove_at_en(vec_ct vec, void *dst, ssize_t pos, size_t n);

/// Destroy and remove n elements at position.
///
/// \param vec      vector
/// \param pos      position of first element, negative value counts from last element
/// \param n        number of elements to remove
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos or \p pos + \p n is out of bounds
int vec_remove_at_fn(vec_ct vec, ssize_t pos, size_t n, vec_dtor_cb dtor, const void *ctx);

/// Find first element matching predicate.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        pointer to element
/// \retval NULL/E_VEC_NOT_FOUND    \p pred did not match any element
void *vec_find(vec_const_ct vec, vec_pred_cb pred, const void *ctx);

/// Find and get first element matching predicate.
///
/// \param      vec     vector
/// \param[out] dst     pointer to element to fill, may be NULL
/// \param      pred    predicate
/// \param      ctx     \p pred context
///
/// \retval 0                   success
/// \retval -1/E_VEC_NOT_FOUND  \p pred did not match any element
int vec_find_e(vec_const_ct vec, void *dst, vec_pred_cb pred, const void *ctx);

/// Find and return first element matching predicate casted to pointer.
///
/// \note Vector element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        element casted to pointer
/// \retval NULL/E_VEC_NOT_FOUND    \p pred did not match any element
void *vec_find_p(vec_const_ct vec, vec_pred_cb pred, const void *ctx);

/// Find last element matching predicate.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        pointer to element
/// \retval NULL/E_VEC_NOT_FOUND    \p pred did not match any element
void *vec_find_r(vec_const_ct vec, vec_pred_cb pred, const void *ctx);

/// Find and get last element matching predicate.
///
/// \param      vec     vector
/// \param[out] dst     pointer to element to fill, may be NULL
/// \param      pred    predicate
/// \param      ctx     \p pred context
///
/// \retval 0                   success
/// \retval -1/E_VEC_NOT_FOUND  \p pred did not match any element
int vec_find_re(vec_const_ct vec, void *dst, vec_pred_cb pred, const void *ctx);

/// Find and return last element matching predicate casted to pointer.
///
/// \note Vector element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        element casted to pointer
/// \retval NULL/E_VEC_NOT_FOUND    \p pred did not match any element
void *vec_find_rp(vec_const_ct vec, vec_pred_cb pred, const void *ctx);

/// Find position of first element matching predicate.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                    position of element
/// \retval -1/E_VEC_NOT_FOUND  \p pred did not match any element
ssize_t vec_find_pos(vec_const_ct vec, vec_pred_cb pred, const void *ctx);

/// Find position of last element matching predicate.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                    position of element
/// \retval -1/E_VEC_NOT_FOUND  \p pred did not match any element
ssize_t vec_find_pos_r(vec_const_ct vec, vec_pred_cb pred, const void *ctx);

/// Find and remove first element matching predicate.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \retval 0                   success
/// \retval -1/E_VEC_NOT_FOUND  \p pred did not match any element
int vec_find_remove(vec_ct vec, vec_pred_cb pred, const void *ctx);

/// Find, get and remove first element matching predicate.
///
/// \param      vec     vector
/// \param[out] dst     pointer to element to fill, may be NULL
/// \param      pred    predicate
/// \param      ctx     \p pred context
///
/// \retval 0                   success
/// \retval -1/E_VEC_NOT_FOUND  \p pred did not match any element
int vec_find_remove_e(vec_ct vec, void *dst, vec_pred_cb pred, const void *ctx);

/// Find, remove and return first element matching predicate casted to pointer.
///
/// \note Vector element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        element casted to pointer
/// \retval NULL/E_VEC_NOT_FOUND    \p pred did not match any element
void *vec_find_remove_p(vec_ct vec, vec_pred_cb pred, const void *ctx);

/// Find, destroy and remove first element matching predicate.
///
/// \param vec          vector
/// \param pred         predicate
/// \param pred_ctx     \p pred context
/// \param dtor         callback to destroy element, may be NULL
/// \param dtor_ctx     \p dtor context
///
/// \retval 0                   success
/// \retval -1/E_VEC_NOT_FOUND  \p pred did not match any element
int vec_find_remove_f(vec_ct vec, vec_pred_cb pred, const void *pred_ctx, vec_dtor_cb dtor, const void *dtor_ctx);

/// Find and remove last element matching predicate.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \retval 0                   success
/// \retval -1/E_VEC_NOT_FOUND  \p pred did not match any element
int vec_find_remove_r(vec_ct vec, vec_pred_cb pred, const void *ctx);

/// Find, get and remove last element matching predicate.
///
/// \param      vec     vector
/// \param[out] dst     pointer to element to fill, may be NULL
/// \param      pred    predicate
/// \param      ctx     \p pred context
///
/// \retval 0                   success
/// \retval -1/E_VEC_NOT_FOUND  \p pred did not match any element
int vec_find_remove_re(vec_ct vec, void *dst, vec_pred_cb pred, const void *ctx);

/// Find, remove and return last element matching predicate casted to pointer.
///
/// \note Vector element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns                        element casted to pointer
/// \retval NULL/E_VEC_NOT_FOUND    \p pred did not match any element
void *vec_find_remove_rp(vec_ct vec, vec_pred_cb pred, const void *ctx);

/// Find, destroy and remove last element matching predicate from vector.
///
/// \param vec          vector
/// \param pred         predicate
/// \param pred_ctx     \p pred context
/// \param dtor         callback to destroy element, may be NULL
/// \param dtor_ctx     \p dtor context
///
/// \retval 0                   success
/// \retval -1/E_VEC_NOT_FOUND  \p pred did not match any element
int vec_find_remove_rf(vec_ct vec, vec_pred_cb pred, const void *pred_ctx, vec_dtor_cb dtor, const void *dtor_ctx);

/// Find and remove all elements matching predicate.
///
/// \param vec      vector
/// \param pred     predicate
/// \param ctx      \p pred context
///
/// \returns        number of elements removed
size_t vec_find_remove_all(vec_ct vec, vec_pred_cb pred, const void *ctx);

/// Find, destroy and remove all elements matching predicate.
///
/// \param vec          vector
/// \param pred         predicate
/// \param pred_ctx     \p pred context
/// \param dtor         callback to destroy element, may be NULL
/// \param dtor_ctx     \p dtor context
///
/// \returns            number of elements removed
size_t vec_find_remove_all_f(vec_ct vec, vec_pred_cb pred, const void *pred_ctx, vec_dtor_cb dtor, const void *dtor_ctx);

/// Swap positions of two elements.
///
/// \param vec      vector
/// \param pos1     position of first element, negative value counts from last element
/// \param pos2     position of second element, negative value counts from last element
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos1 or \p pos2 is out of bounds
int vec_swap(vec_ct vec, ssize_t pos1, ssize_t pos2);

/// Get and remove vector buffer.
///
/// After removing the buffer it is unset until the next push/insert operation.
/// The new buffer is allocated with the minimum capacity.
///
/// \param      vec         vector
/// \param[out] buf         buffer pointer to set
/// \param[out] size        buffer size to set, may be NULL
/// \param[out] capacity    buffer capacity to set, may be NULL
///
/// \retval 0                   success
/// \retval -1/E_VEC_NO_BUFFER  no buffer allocated
int vec_get_buffer(vec_ct vec, void **buf, size_t *size, size_t *capacity);

/// Truncate vector to n elements.
///
/// \param vec      vector
/// \param size     number of elements to keep
///
/// \returns        number of elements removed
size_t vec_truncate(vec_ct vec, size_t size);

/// Truncate vector to n elements. Destroy truncated elements.
///
/// \param vec      vector
/// \param size     number of elements to keep
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \returns        number of elements removed
size_t vec_truncate_f(vec_ct vec, size_t size, vec_dtor_cb dtor, const void *ctx);

/// Set vector capacity.
///
/// \param vec          vector
/// \param capacity     capacity to set, at least minimum capacity is set
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int vec_set_capacity(vec_ct vec, size_t capacity);

/// Set vector capacity. Destroy truncated elements.
///
/// \param vec          vector
/// \param capacity     capacity to set, at least minimum capacity is set
/// \param dtor         callback to destroy element, may be NULL
/// \param ctx          \p dtor context
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int vec_set_capacity_f(vec_ct vec, size_t capacity, vec_dtor_cb dtor, const void *ctx);

/// Fold over all elements, starting with first element.
///
/// \param vec      vector
/// \param fold     callback to invoke on each element
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval <0/E_VEC_CALLBACK   \p fold error
/// \retval >0                  \p fold rc
int vec_fold(vec_const_ct vec, vec_fold_cb fold, const void *ctx);

/// Fold over all elements, starting with last element.
///
/// \param vec      vector
/// \param fold     callback to invoke on each element
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval <0/E_VEC_CALLBACK   \p fold error
/// \retval >0                  \p fold rc
int vec_fold_r(vec_const_ct vec, vec_fold_cb fold, const void *ctx);

/// Sort vector.
///
/// \param vec      vector
/// \param sort     callback to compare two elements
/// \param ctx      \p sort context
void vec_sort(vec_ct vec, vec_sort_cb sort, const void *ctx);


#endif // ifndef YTIL_CON_VEC_H_INCLUDED
