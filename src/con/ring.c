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

#include <ytil/con/ring.h>
#include <ytil/def.h>
#include <ytil/def/magic.h>
#include <ytil/ext/string.h>
#include <stdlib.h>


/// Get pointer to ring element.
///
/// \param p    position of element
///
/// \returns    pointer to element
#define ELEM(p) (ring->mem + (p)*ring->esize)

/// Get real position from OOB position.
///
/// \param p    OOB position
///
/// \returns    element position
#define POS(p)  ((p) % ring->cap)

/// Get tail ring element.
///
/// \returns    pointer to element
#define TAIL()  ELEM(ring->tail)

/// Get head ring element.
///
/// \returns    pointer to element
#define HEAD()  ELEM(POS(ring->tail + ring->size -1))

/// Get next ring element position.
///
/// \param p    position
///
/// \returns    element position +1
#define NEXT(p) POS((p) +1)

/// Get previous ring element position.
///
/// \param p    position
///
/// \returns    element position -1
#define PREV(p) POS((p) + ring->cap -1)


#define MAGIC       define_magic("RIN")     ///< ring magic
#define DEFAULT_CAP 10                      ///< default capacity


/// ring
typedef struct ring
{
    DEBUG_MAGIC

    char    *mem;       ///< ring memory
    size_t  esize;      ///< element size
    size_t  cap;        ///< number of allocated elements
    size_t  tail;       ///< position of tail element
    size_t  size;       ///< number of elements
} ring_st;

/// ring error type definition
ERROR_DEFINE_LIST(RING,
    ERROR_INFO(E_RING_CALLBACK,     "Callback error."),
    ERROR_INFO(E_RING_EMPTY,        "Ring is empty."),
    ERROR_INFO(E_RING_FULL,         "Ring is full.")
);

/// default error type for ring module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_RING


ring_ct ring_new(size_t elemsize)
{
    return error_pass_ptr(ring_new_c(0, elemsize));
}

ring_ct ring_new_c(size_t capacity, size_t elemsize)
{
    ring_ct ring;

    assert(elemsize);

    if(!(ring = calloc(1, sizeof(ring_st))))
        return error_wrap_last_errno(calloc), NULL;

    init_magic(ring);
    ring->esize = elemsize;
    ring->cap   = capacity ? capacity : DEFAULT_CAP;

    return ring;
}

void ring_free(ring_ct ring)
{
    ring_free_f(ring, NULL, NULL);
}

void ring_free_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx)
{
    ring_clear_f(ring, dtor, ctx);

    if(ring->mem)
        free(ring->mem);

    free(ring);
}

ring_ct ring_free_if_empty(ring_ct ring)
{
    return ring_free_if_empty_f(ring, NULL, NULL);
}

ring_ct ring_free_if_empty_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx)
{
    assert_magic(ring);

    if(ring->size)
        return ring;

    ring_free_f(ring, dtor, ctx);

    return NULL;
}

void ring_clear(ring_ct ring)
{
    ring_clear_f(ring, NULL, NULL);
}

void ring_clear_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx)
{
    size_t tail, size;

    assert_magic(ring);
    return_if_fail(ring->mem);

    if(dtor)
    {
        for(tail = ring->tail, size = ring->size; size; tail = NEXT(tail), size--)
            dtor(ring, ELEM(tail), (void *)ctx);
    }

    ring->size = 0;
}

ring_ct ring_clone(ring_const_ct ring)
{
    return error_pass_ptr(ring_clone_f(ring, NULL, NULL, NULL));
}

ring_ct ring_clone_f(ring_const_ct ring, ring_clone_cb clone, ring_dtor_cb dtor, const void *ctx)
{
    ring_ct ring2;
    size_t tail, size;

    assert_magic(ring);

    if(!(ring2 = ring_new_c(ring->cap, ring->esize)))
        return error_pass(), NULL;

    return_value_if_fail(ring->mem, ring2);

    ring2->tail = ring->tail;
    ring2->size = ring->size;

    if(!clone)
    {
        if(!(ring2->mem = memdup(ring->mem, ring->cap * ring->esize)))
            return error_wrap_last_errno(memdup), free(ring2), NULL;

        return ring2;
    }

    if(!(ring2->mem = calloc(ring->cap, ring->esize)))
        return error_wrap_last_errno(calloc), free(ring2), NULL;

    for(tail = ring->tail, size = ring->size; size; tail = NEXT(tail), size--)
    {
        if(clone(ring, ring2->mem + tail * ring->esize, ELEM(tail), (void *)ctx))
            return error_pack(E_RING_CALLBACK), ring_free_f(ring2, dtor, ctx), NULL;
    }

    return ring2;
}

bool ring_is_empty(ring_const_ct ring)
{
    try_magic(ring);

    return !ring || !ring->size;
}

size_t ring_size(ring_const_ct ring)
{
    try_magic(ring);

    return ring ? ring->size : 0;
}

size_t ring_elemsize(ring_const_ct ring)
{
    assert_magic(ring);

    return ring->esize;
}

size_t ring_capacity(ring_const_ct ring)
{
    assert_magic(ring);

    return ring->cap;
}

size_t ring_memsize(ring_const_ct ring)
{
    return ring_memsize_f(ring, NULL, NULL);
}

size_t ring_memsize_f(ring_const_ct ring, ring_size_cb sizef, const void *ctx)
{
    size_t bytes, tail, size;

    assert_magic(ring);

    bytes = sizeof(ring_st) + ring->cap * ring->esize;

    if(ring->mem && sizef)
    {
        for(tail = ring->tail, size = ring->size; size; tail = NEXT(tail), size--)
            bytes += sizef(ring, ELEM(tail), (void *)ctx);
    }

    return bytes;
}

void *ring_put(ring_ct ring)
{
    assert_magic(ring);
    return_error_if_fail(ring->size < ring->cap, E_RING_FULL, NULL);

    return error_pass_ptr(ring_put_overwrite_e(ring, NULL, NULL, NULL));
}

void *ring_put_e(ring_ct ring, const void *elem)
{
    assert_magic(ring);
    return_error_if_fail(ring->size < ring->cap, E_RING_FULL, NULL);

    return error_pass_ptr(ring_put_overwrite_e(ring, elem, NULL, NULL));
}

void *ring_put_p(ring_ct ring, const void *ptr)
{
    assert_magic(ring);
    assert(ring->esize == sizeof(void *));
    return_error_if_fail(ring->size < ring->cap, E_RING_FULL, NULL);

    return error_pass_ptr(ring_put_overwrite_e(ring, &ptr, NULL, NULL));
}

void *ring_put_overwrite(ring_ct ring, ring_dtor_cb dtor, const void *ctx)
{
    return error_pass_ptr(ring_put_overwrite_e(ring, NULL, dtor, ctx));
}

void *ring_put_overwrite_e(ring_ct ring, const void *elem, ring_dtor_cb dtor, const void *ctx)
{
    void *head;

    assert_magic(ring);

    if(!ring->mem && !(ring->mem = calloc(ring->cap, ring->esize)))
        return error_wrap_last_errno(calloc), NULL;

    if(ring->size == ring->cap)
    {
        if(dtor)
            dtor(ring, TAIL(), (void *)ctx);

        ring->tail = NEXT(ring->tail);
        ring->size--;
    }

    ring->size++;
    head = HEAD();

    if(elem)
        memcpy(head, elem, ring->esize);
    else
        memset(head, 0, ring->esize);

    return head;
}

void *ring_put_overwrite_p(ring_ct ring, const void *ptr, ring_dtor_cb dtor, const void *ctx)
{
    assert_magic(ring);
    assert(ring->esize == sizeof(void *));

    return error_pass_ptr(ring_put_overwrite_e(ring, &ptr, dtor, ctx));
}

void *ring_peek(ring_const_ct ring)
{
    assert_magic(ring);
    return_error_if_fail(ring->size, E_RING_EMPTY, NULL);

    return TAIL();
}

void *ring_peek_p(ring_const_ct ring)
{
    assert_magic(ring);
    assert(ring->esize == sizeof(void *));
    return_error_if_fail(ring->size, E_RING_EMPTY, NULL);

    return *(void **)TAIL();
}

int ring_drop(ring_ct ring)
{
    return error_pass_int(ring_drop_f(ring, NULL, NULL));
}

int ring_drop_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx)
{
    assert_magic(ring);
    return_error_if_fail(ring->size, E_RING_EMPTY, -1);

    if(dtor)
        dtor(ring, TAIL(), (void *)ctx);

    ring->tail = NEXT(ring->tail);
    ring->size--;

    return 0;
}

int ring_get(ring_ct ring, void *elem)
{
    assert_magic(ring);
    return_error_if_fail(ring->size, E_RING_EMPTY, -1);

    if(elem)
        memcpy(elem, TAIL(), ring->esize);

    ring->tail = NEXT(ring->tail);
    ring->size--;

    return 0;
}

void *ring_get_p(ring_ct ring)
{
    void *e;

    assert_magic(ring);
    assert(ring->esize == sizeof(void *));
    return_error_if_fail(ring->size, E_RING_EMPTY, NULL);

    e = *(void **)TAIL();

    ring->tail = NEXT(ring->tail);
    ring->size--;

    return e;
}

void *ring_peek_head(ring_const_ct ring)
{
    assert_magic(ring);
    return_error_if_fail(ring->size, E_RING_EMPTY, NULL);

    return HEAD();
}

void *ring_peek_head_p(ring_const_ct ring)
{
    assert_magic(ring);
    assert(ring->esize == sizeof(void *));
    return_error_if_fail(ring->size, E_RING_EMPTY, NULL);

    return *(void **)HEAD();
}

int ring_drop_head(ring_ct ring)
{
    return error_pass_int(ring_drop_head_f(ring, NULL, NULL));
}

int ring_drop_head_f(ring_ct ring, ring_dtor_cb dtor, const void *ctx)
{
    assert_magic(ring);
    return_error_if_fail(ring->size, E_RING_EMPTY, -1);

    if(dtor)
        dtor(ring, HEAD(), (void *)ctx);

    ring->size--;

    return 0;
}

int ring_get_head(ring_ct ring, void *elem)
{
    assert_magic(ring);
    return_error_if_fail(ring->size, E_RING_EMPTY, -1);

    if(elem)
        memcpy(elem, HEAD(), ring->esize);

    ring->size--;

    return 0;
}

void *ring_get_head_p(ring_ct ring)
{
    void *e;

    assert_magic(ring);
    assert(ring->esize == sizeof(void *));
    return_error_if_fail(ring->size, E_RING_EMPTY, NULL);

    e = *(void **)HEAD();

    ring->size--;

    return e;
}

int ring_fold(ring_ct ring, ring_fold_cb fold, const void *ctx)
{
    size_t tail, size;
    int rc;

    assert_magic(ring);
    assert(fold);

    for(tail = ring->tail, size = ring->size; size; tail = NEXT(tail), size--)
    {
        if((rc = fold(ring, ELEM(tail), (void *)ctx)))
            return error_pack_int(E_RING_CALLBACK, rc);
    }

    return 0;
}

int ring_fold_r(ring_ct ring, ring_fold_cb fold, const void *ctx)
{
    size_t head, size;
    int rc;

    assert_magic(ring);
    assert(fold);

    for(head = POS(ring->tail + ring->size -1), size = ring->size; size; head = PREV(head), size--)
    {
        if((rc = fold(ring, ELEM(head), (void *)ctx)))
            return error_pack_int(E_RING_CALLBACK, rc);
    }

    return 0;
}
