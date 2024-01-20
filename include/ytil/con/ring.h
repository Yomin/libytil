/*
 * Copyright (c) 2016-2024 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_CON_RING_H_INCLUDED
#define YTIL_CON_RING_H_INCLUDED

#include <stddef.h>
#include <stdbool.h>
#include <ytil/gen/error.h>


/// ring error
typedef enum ring_error
{
    E_RING_CALLBACK,        ///< callback error
    E_RING_EMPTY,           ///< ring is empty
    E_RING_FULL,            ///< ring is full
} ring_error_id;

/// ring error type declaration
ERROR_DECLARE(RING);


struct ring;

typedef       struct ring   *ring_ct;           ///< ring type
typedef const struct ring   *ring_const_ct;     ///< const ring type


/// ring element size callback
///
/// \param ring     ring
/// \param elem     element to get size of
/// \param ctx      callback context
///
/// \returns        size of element
typedef size_t (*ring_size_cb)(ring_const_ct ring, const void *elem, void *ctx);

/// ring element dtor callback
///
/// \param ring     ring
/// \param elem     element to destroy
/// \param ctx      callback context
typedef void (*ring_dtor_cb)(ring_const_ct ring, void *elem, void *ctx);

/// ring element fold callback
///
/// \param ring     ring
/// \param elem     element
/// \param ctx      callback context
///
/// \retval 0       continue fold
/// \retval <0      stop fold with error
/// \retval >0      stop fold
typedef int (*ring_fold_cb)(ring_const_ct ring, void *elem, void *ctx);

/// ring element clone callback
///
/// \param      src_ring    source ring
/// \param[out] dst         element to fill
/// \param      src         element to copy
/// \param      ctx         callback context
///
/// \retval 0               success
/// \retval !=0             error
typedef int (*ring_clone_cb)(ring_const_ct src_ring, void *dst, const void *src, void *ctx);


/// Create new ring of default capacity.
///
/// \param elemsize     element size
///
/// \returns                    new ring
/// \retval NULL/E_GENERIC_OOM  out of memory
ring_ct ring_new(size_t elemsize);

/// Create new ring.
///
/// \param capacity     ring size in number of elements
/// \param elemsize     element size
///
/// \returns                    new ring
/// \retval NULL/E_GENERIC_OOM  out of memory
ring_ct ring_new_c(size_t capacity, size_t elemsize);

/// Free ring.
///
/// \param ring     ring
void ring_free(ring_ct ring);

/// Destroy elements and free ring.
///
/// \param ring     ring
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
void ring_free_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx);

/// Free ring if empty.
///
/// \param ring     ring
///
/// \retval ring    ring is not empty
/// \retval NULL    ring was empty and was freed
ring_ct ring_free_if_empty(ring_ct ring);

/// Destroy elements and free ring if empty.
///
/// \param ring     ring
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval ring    ring is not empty
/// \retval NULL    ring was empty and was freed
ring_ct ring_free_if_empty_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx);

/// Remove all elements from ring.
///
/// \param ring     ring
void ring_clear(ring_ct ring);

/// Destroy and remove all elements from ring.
///
/// \param ring     ring
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
void ring_clear_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx);

/// Clone ring.
///
/// \param ring     ring
///
/// \returns                        new ring
/// \retval NULL/E_GENERIC_OOM      out of memory
ring_ct ring_clone(ring_const_ct ring);

/// Clone ring with clone callback.
///
/// If clone callback is given, invoke clone on each element,
/// else memdup each element.
/// If cloning fails and dtor callback is given,
/// invoke dtor on each already cloned element.
///
/// \param ring     ring
/// \param clone    callback to duplicate element, may be NULL
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p clone and \p dtor context
///
/// \returns                        new ring
/// \retval NULL/E_GENERIC_OOM      out of memory
/// \retval NULL/E_RING_CALLBACK    \p clone callback error
ring_ct ring_clone_f(ring_const_ct ring, ring_clone_cb clone, ring_dtor_cb dtor, const void *ctx);

/// Check if ring is NULL or empty.
///
/// \param ring     ring, may be NULL
///
/// \retval true    ring is NULL or empty
/// \retval false   ring is not empty
bool ring_is_empty(ring_const_ct ring);

/// Get number of ring elements.
///
/// \param ring     ring, may be NULL
///
/// \returns        number of ring elements
size_t ring_size(ring_const_ct ring);

/// Get ring element size.
///
/// \param ring     ring
///
/// \returns        element size
size_t ring_elemsize(ring_const_ct ring);

/// Get ring capacity.
///
/// \param ring     ring
///
/// \returns        capacity in number of elements
size_t ring_capacity(ring_const_ct ring);

/// Get allocated size of ring.
///
/// \param ring     ring
///
/// \returns        allocated size in bytes
size_t ring_memsize(ring_const_ct ring);

/// Get allocated size of ring and elements.
///
/// The returned size of \p size callback is added to the element size.
///
/// \param ring     ring
/// \param size     callback to determine size of element, may be NULL
/// \param ctx      \p size context
///
/// \returns        allocated size in bytes
size_t ring_memsize_f(ring_const_ct ring, ring_size_cb size, const void *ctx);

/// Put empty element into ring.
///
/// \param ring     ring
///
/// \returns                    pointer to new element
/// \retval NULL/E_RING_FULL    ring is full
/// \retval NULL/E_GENERIC_OOM  out of memory
void *ring_put(ring_ct ring);

/// Put element into ring.
///
/// \param ring     ring
/// \param elem     pointer to element, may be NULL
///
/// \returns                    pointer to new element
/// \retval NULL/E_RING_FULL    ring is full
/// \retval NULL/E_GENERIC_OOM  out of memory
void *ring_put_e(ring_ct ring, const void *elem);

/// Put pointer into ring.
///
/// \note ring element size must be equal to sizeof(void*).
///
/// \param ring     ring
/// \param ptr      pointer
///
/// \returns                    pointer to new element
/// \retval NULL/E_RING_FULL    ring is full
/// \retval NULL/E_GENERIC_OOM  out of memory
void *ring_put_p(ring_ct ring, const void *ptr);

/// Put empty element into ring, overwrite tail if full.
///
/// \param ring     ring
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \returns                    pointer to new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *ring_put_overwrite(ring_ct ring, ring_dtor_cb dtor, const void *ctx);

/// Put element into ring, overwrite tail if full.
///
/// \param ring     ring
/// \param elem     pointer to element, may be NULL
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \returns                    pointer to new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *ring_put_overwrite_e(ring_ct ring, const void *elem, ring_dtor_cb dtor, const void *ctx);

/// Put pointer into ring, overwrite tail if full.
///
/// \note ring element size must be equal to sizeof(void*).
///
/// \param ring     ring
/// \param ptr      pointer
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \returns                    pointer to new element
/// \retval NULL/E_GENERIC_OOM  out of memory
void *ring_put_overwrite_p(ring_ct ring, const void *ptr, ring_dtor_cb dtor, const void *ctx);

/// Get pointer to tail ring element.
///
/// \param ring     ring
///
/// \returns                    pointer to element
/// \retval NULL/E_RING_EMPTY   ring is empty
void *ring_peek(ring_const_ct ring);

/// Get tail ring element casted to pointer.
///
/// \note ring element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param ring     ring
///
/// \returns                    element casted to pointer
/// \retval NULL/E_RING_EMPTY   ring is empty
void *ring_peek_p(ring_const_ct ring);

/// Remove tail ring element.
///
/// \param ring     ring
///
/// \retval 0               success
/// \retval -1/E_RING_EMPTY ring is empty
int ring_drop(ring_ct ring);

/// Destroy and remove tail ring element.
///
/// \param ring     ring
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0               success
/// \retval -1/E_RING_EMPTY ring is empty
int ring_drop_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx);

/// Get and remove tail ring element.
///
/// \param      ring    ring
/// \param[out] dst     pointer to element to fill, may be NULL
///
/// \retval 0               success
/// \retval -1/E_RING_EMPTY ring is empty
int ring_get(ring_ct ring, void *dst);

/// Remove and return tail ring element casted to pointer.
///
/// \note ring element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param ring     ring
///
/// \returns                    element casted to pointer
/// \retval NULL/E_RING_EMPTY   ring is empty
void *ring_get_p(ring_ct ring);

/// Get pointer to head ring element.
///
/// \param ring     ring
///
/// \returns                    pointer to element
/// \retval NULL/E_RING_EMPTY   ring is empty
void *ring_peek_head(ring_const_ct ring);

/// Get head ring element casted to pointer.
///
/// \note ring element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param ring     ring
///
/// \returns                    element casted to pointer
/// \retval NULL/E_RING_EMPTY   ring is empty
void *ring_peek_head_p(ring_const_ct ring);

/// Remove head ring element.
///
/// \param ring     ring
///
/// \retval 0               success
/// \retval -1/E_RING_EMPTY ring is empty
int ring_drop_head(ring_ct ring);

/// Destroy and remove head ring element.
///
/// \param ring     ring
/// \param dtor     callback to destroy element, may be NULL
/// \param ctx      \p dtor context
///
/// \retval 0               success
/// \retval -1/E_RING_EMPTY ring is empty
int ring_drop_head_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx);

/// Get and remove head ring element.
///
/// \param      ring    ring
/// \param[out] dst     pointer to element to fill, may be NULL
///
/// \retval 0               success
/// \retval -1/E_RING_EMPTY ring is empty
int ring_get_head(ring_ct ring, void *dst);

/// Remove and return head ring element casted to pointer.
///
/// \note ring element size must be equal to sizeof(void*).
/// \note To distinguish a stored NULL pointer from an error,
///       call error_clear() before calling this function
///       and evaluate e.g. error_depth() afterwards.
///
/// \param ring     ring
///
/// \returns                    element casted to pointer
/// \retval NULL/E_RING_EMPTY   ring is empty
void *ring_get_head_p(ring_ct ring);

/// Fold over all elements, starting with tail element.
///
/// \param ring     ring
/// \param fold     callback to invoke on each element
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval <0/E_RING_CALLBACK  \p fold error
/// \retval >0                  \p fold rc
int ring_fold(ring_ct ring, ring_fold_cb fold, const void *ctx);

/// Fold over all elements, starting with head element.
///
/// \param ring     ring
/// \param fold     callback to invoke on each element
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval <0/E_RING_CALLBACK  \p fold error
/// \retval >0                  \p fold rc
int ring_fold_r(ring_ct ring, ring_fold_cb fold, const void *ctx);


#endif // ifndef YTIL_CON_RING_H_INCLUDED
