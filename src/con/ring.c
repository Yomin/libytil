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

#include <ytil/con/ring.h>
#include <ytil/def.h>
#include <ytil/def/magic.h>
#include <stdlib.h>
#include <string.h>


#define ELEM(e) (ring->mem + (e)*ring->esize)
#define POS(p)  ((p) % ring->cap)
#define TAIL()  ELEM(ring->tail)
#define HEAD()  ELEM(POS(ring->tail + ring->size -1))
#define DEC(p)  POS((p) + ring->cap -1)
#define INC(p)  POS((p) +1)

#define MAGIC       define_magic("RIN")
#define DEFAULT_CAP 10

typedef struct ring
{
    DEBUG_MAGIC
    
    unsigned char *mem;
    size_t esize, cap, tail, size;
} ring_st;

/// ring error type definition
ERROR_DEFINE_LIST(RING,
      ERROR_INFO(E_RING_CALLBACK, "Callback error.")
    , ERROR_INFO(E_RING_EMPTY, "Ring is empty.")
    , ERROR_INFO(E_RING_INVALID_ELEMSIZE, "Invalid element size.")
    , ERROR_INFO(E_RING_NO_SPACE, "No space to put element into available.")
);

/// default error type for ring module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_RING


ring_ct ring_new(size_t capacity, size_t elemsize)
{
    ring_ct ring;
    
    return_error_if_fail(elemsize, E_RING_INVALID_ELEMSIZE, NULL);
    
    if(!(ring = calloc(1, sizeof(ring_st))))
        return error_wrap_last_errno(calloc), NULL;
    
    init_magic(ring);
    ring->esize = elemsize;
    ring->cap = capacity ? capacity : DEFAULT_CAP;
    
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
    assert_magic(ring);
    
    if(ring->size)
        return ring;
    
    ring_free_f(ring, NULL, NULL);
    
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
        for(tail=ring->tail, size=ring->size; size; tail=INC(tail), size--)
            dtor(ring, ELEM(tail), (void *)ctx);
    
    ring->size = 0;
}

bool ring_is_empty(ring_const_ct ring)
{
    assert_magic(ring);
    
    return !ring->size;
}

size_t ring_size(ring_const_ct ring)
{
    assert_magic(ring);
    
    return ring->size;
}

void *ring_put(ring_ct ring)
{
    return error_pass_ptr(ring_put_ef(ring, NULL, NULL, NULL));
}

void *ring_put_e(ring_ct ring, const void *elem)
{
    return error_pass_ptr(ring_put_ef(ring, elem, NULL, NULL));
}

void *ring_put_f(ring_ct ring, ring_overflow_cb overflow, const void *ctx)
{
    return error_pass_ptr(ring_put_ef(ring, NULL, overflow, ctx));
}

void *ring_put_ef(ring_ct ring, const void *elem, ring_overflow_cb overflow, const void *ctx)
{
    void *head;
    
    assert_magic(ring);
    
    if(!ring->mem && !(ring->mem = calloc(ring->cap, ring->esize)))
        return error_wrap_last_errno(calloc), NULL;
    
    if(ring->size == ring->cap)
    {
        if(!overflow || overflow(ring, elem, TAIL(), (void *)ctx) == RING_REJECT)
            return error_set(E_RING_NO_SPACE), NULL;
        else
        {
            ring->tail = INC(ring->tail);
            ring->size--;
        }
    }
    
    ring->size++;
    head = HEAD();
    
    if(elem)
        memcpy(head, elem, ring->esize);
    else
        memset(head, 0, ring->esize);
    
    return head;
}

void *ring_peek(ring_const_ct ring)
{
    assert_magic(ring);
    return_error_if_fail(ring->size, E_RING_EMPTY, NULL);
    
    return TAIL();
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
    
    ring->tail = INC(ring->tail);
    ring->size--;
    
    return 0;
}

int ring_get(ring_ct ring, void *elem)
{
    assert_magic(ring);
    return_error_if_fail(ring->size, E_RING_EMPTY, -1);
    
    if(elem)
        memcpy(elem, TAIL(), ring->esize);
    
    ring->tail = INC(ring->tail);
    ring->size--;
    
    return 0;
}

void *ring_peek_head(ring_const_ct ring)
{
    assert_magic(ring);
    return_error_if_fail(ring->size, E_RING_EMPTY, NULL);
    
    return HEAD();
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

int ring_fold(ring_ct ring, ring_fold_cb fold, const void *ctx)
{
    size_t tail, size;
    int rc;
    
    assert_magic(ring);
    assert(fold);
    
    for(tail=ring->tail, size=ring->size; size; tail=INC(tail), size--)
        if((rc = fold(ring, ELEM(tail), (void *)ctx)))
            return error_pack_int(E_RING_CALLBACK, rc);
    
    return 0;
}

int ring_fold_r(ring_ct ring, ring_fold_cb fold, const void *ctx)
{
    size_t head, size;
    int rc;
    
    assert_magic(ring);
    assert(fold);
    
    for(head=POS(ring->tail+ring->size-1), size=ring->size; size; head=DEC(head), size--)
        if((rc = fold(ring, ELEM(head), (void *)ctx)))
            return error_pack_int(E_RING_CALLBACK, rc);
    
    return 0;
}
