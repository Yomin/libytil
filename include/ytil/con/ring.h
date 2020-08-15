/*
 * Copyright (c) 2016-2019 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_CON_RING_H_INCLUDED
#define YTIL_CON_RING_H_INCLUDED

#include <stddef.h>
#include <stdbool.h>


typedef enum ring_error
{
      E_RING_CALLBACK
    , E_RING_EMPTY
    , E_RING_INVALID_ELEMSIZE
    , E_RING_NO_SPACE
} ring_error_id;

typedef enum ring_action
{
      RING_REJECT
    , RING_OVERWRITE
} ring_action_id;

struct ring;

typedef       struct ring *ring_ct;
typedef const struct ring *ring_const_ct;

// free ring elem
typedef void (*ring_dtor_cb)(ring_const_ct ring, void *elem, void *ctx);
// return 0 to continue fold, anything else stops fold
typedef int  (*ring_fold_cb)(ring_const_ct ring, void *elem, void *ctx);
// duplicate src ring elem into dst, return 0 on success
typedef int  (*ring_clone_cb)(ring_const_ct ring, void *dst, const void *src, void *ctx);
// decide on overflow whether to reject the new or overwrite the existing (tail) elem
// if overwriting the existing elem dont forget to free it if necessary
typedef ring_action_id (*ring_overflow_cb)(ring_const_ct ring, const void *new_elem, void *old_elem, void *ctx);


// create new ring
ring_ct ring_new(size_t capacity, size_t elemsize);
// free ring
void    ring_free(ring_ct ring);
// free ring, apply dtor on each elem
void    ring_free_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx);
// free ring if empty
ring_ct ring_free_if_empty(ring_ct ring);

// remove all elems from ring
void ring_clear(ring_ct ring);
// remove all elems from ring, apply dtor on each elem
void ring_clear_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx);

// duplicate ring
ring_ct ring_clone(ring_const_ct ring);
// duplicate ring, duplicate elem with clone function
// in case the clone fails, already duplicated elems are freed with dtor
ring_ct ring_clone_f(ring_const_ct ring, ring_clone_cb clone, ring_dtor_cb dtor, const void *ctx);

// check if ring is empty
bool   ring_is_empty(ring_const_ct ring);
// get ring elem count
size_t ring_size(ring_const_ct ring);

// put empty elem into ring
void *ring_put(ring_ct ring);
// put elem into ring
void *ring_put_e(ring_ct ring, const void *elem);
// put empty elem into ring, handle overflow
void *ring_put_f(ring_ct ring, ring_overflow_cb overflow, const void *ctx);
// put elem into ring, handle overflow
void *ring_put_ef(ring_ct ring, const void *elem, ring_overflow_cb overflow, const void *ctx);

// return pointer to current tail elem
void *ring_peek(ring_const_ct ring);
// remove current tail elem from ring
int   ring_drop(ring_ct ring);
// apply dtor to current tail elem and remove it from ring
int   ring_drop_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx);
// copy current tail elem into dst and remove it from ring
int   ring_get(ring_ct ring, void *dst);

// return pointer to current head elem
void *ring_peek_head(ring_const_ct ring);
// remove current head elem from ring
int   ring_drop_head(ring_ct ring);
// apply dtor to current head elem and remove it from ring
int   ring_drop_head_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx);
// copy current head elem into dst and remove it from ring
int   ring_get_head(ring_ct ring, void *dst);

// apply fold to every ring elem, begin with tail elem
int ring_fold(ring_ct ring, ring_fold_cb fold, const void *ctx);
// apply fold to every ring elem, begin with head elem
int ring_fold_r(ring_ct ring, ring_fold_cb fold, const void *ctx);

#endif
