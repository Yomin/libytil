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

#define _GNU_SOURCE

#include <ytil/con/vec.h>
#include <ytil/def.h>
#include <ytil/magic.h>
#include <ytil/ext/stdlib.h>
#include <string.h>

/// offset of first vector element
#define OFFSET PROD_DEBUG(0, vec->esize)

/// Get position of vector element.
///
/// \param e    element to get position of
///
/// \returns    element position
#define POS(e) (((char *)e - vec->mem) / vec->esize)

/// Get pointer to element.
///
/// \param i    position of element
///
/// \returns    pointer to element
#define ELEM(i) (vec->mem + (i) * vec->esize)


#define MAGIC           define_magic("VEC")     ///< vector magic
#define DEFAULT_CAP     10                      ///< default capacity
#define RESIZE_FACTOR   2                       ///< resize factor


/// vector
typedef struct vector
{
    DEBUG_MAGIC

    char    *mem;       ///< vector memory
    size_t  size;       ///< number of elements
    size_t  esize;      ///< element size
    size_t  cap;        ///< number of allocated elements
    size_t  min_cap;    ///< minimum number of allocated elements
} vec_st;

/// vector error type definition
ERROR_DEFINE_LIST(VEC,
    ERROR_INFO(E_VEC_CALLBACK,      "Callback error."),
    ERROR_INFO(E_VEC_EMPTY,         "Vector is empty."),
    ERROR_INFO(E_VEC_NO_BUFFER,     "No buffer available."),
    ERROR_INFO(E_VEC_NOT_FOUND,     "Element not found."),
    ERROR_INFO(E_VEC_OUT_OF_BOUNDS, "Out of bounds access.")
);

/// default error type for vector module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_VEC


/// Set vector buffer.
///
/// \param vec          vector to set
/// \param mem          memory to set
/// \param capacity     capacity of \p mem in number of elements
/// \param size         size of \p mem in number of elements
static inline void vec_set_buf(vec_ct vec, char *mem, size_t capacity, size_t size)
{
    if(!mem)
    {
        vec->mem    = NULL;
        vec->cap    = 0;
        vec->size   = 0;
    }
    else
    {
        vec->mem    = mem + OFFSET;
        vec->cap    = capacity;
        vec->size   = size;
    }
}

/// Resize vector buffer.
///
/// \param vec          vector
/// \param capacity     capacity to set
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int vec_resize(vec_ct vec, size_t capacity)
{
    char *mem;

    if(!vec->mem)
    {
        if(!(mem = calloc(1, OFFSET + capacity * vec->esize)))
            return error_wrap_errno(calloc), -1;

        vec_set_buf(vec, mem, capacity, 0);
    }
    else
    {
        mem = vec->mem - OFFSET;

        if(!(mem = realloc(mem, OFFSET + capacity * vec->esize)))
        {
            if(capacity > vec->cap)
                return error_wrap_errno(realloc), -1;
            else
                return 0;
        }

        vec_set_buf(vec, mem, capacity, vec->size);
    }

    return 0;
}

/// Grow vector capacity if less than n elements available.
///
/// \param vec      vector
/// \param n        number of elements to fit
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static inline int vec_grow(vec_ct vec, size_t n)
{
    if(  vec->size + n > vec->cap
      && vec_resize(vec, MAX3(vec->min_cap, vec->cap + n, vec->cap * RESIZE_FACTOR)))
        return error_pass(), -1;

    return 0;
}

/// Shrink vector capacity if fill level too low.
///
/// \param vec      vector
static inline void vec_shrink(vec_ct vec)
{
    if(  vec->cap > vec->min_cap
      && vec->size <= vec->cap / (RESIZE_FACTOR * 2))
        vec_resize(vec, MAX(vec->cap / RESIZE_FACTOR, vec->min_cap));
}

/// Check if element is within vector memory and aligned on element size.
///
/// \param vec      vector
/// \param velem    element to check
///
/// \retval 0                       element is within vector memory
/// \retval -1/E_VEC_OUT_OF_BOUNDS  element is not within vector memory
static int vec_check_range(vec_const_ct vec, const void *velem)
{
    const char *elem = velem;

    return_error_if_fail(vec->mem, E_VEC_OUT_OF_BOUNDS, -1);
    return_error_if_fail(elem >= vec->mem && elem < ELEM(vec->size), E_VEC_OUT_OF_BOUNDS, -1);

    assert((elem - vec->mem) % vec->esize == 0);

    return 0;
}

vec_ct vec_new(size_t capacity, size_t elemsize)
{
    vec_ct vec;

    assert(elemsize);

    if(!(vec = calloc(1, sizeof(struct vector))))
        return error_wrap_errno(calloc), NULL;

    init_magic(vec);
    vec->esize      = elemsize;
    vec->min_cap    = capacity ? capacity : DEFAULT_CAP;

    return vec;
}

void vec_free(vec_ct vec)
{
    vec_free_f(vec, NULL, NULL);
}

void vec_free_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx)
{
    size_t i;

    assert_magic(vec);

    if(vec->mem)
    {
        if(dtor)
        {
            for(i = 0; i < vec->size; i++)
                dtor(vec, ELEM(i), (void *)ctx);
        }

        free(vec->mem - OFFSET);
    }

    free(vec);
}

vec_ct vec_free_if_empty(vec_ct vec)
{
    return vec_free_if_empty_f(vec, NULL, NULL);
}

vec_ct vec_free_if_empty_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx)
{
    assert_magic(vec);

    if(vec->size)
        return vec;

    vec_free_f(vec, dtor, ctx);

    return NULL;
}

void vec_clear(vec_ct vec)
{
    vec_clear_f(NULL, NULL, vec);
}

void vec_clear_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx)
{
    size_t i;

    assert_magic(vec);
    return_if_fail(vec->mem);

    if(dtor)
    {
        for(i = 0; i < vec->size; i++)
            dtor(vec, ELEM(i), (void *)ctx);
    }

    vec->size = 0;
    vec_resize(vec, vec->min_cap);
}

vec_ct vec_clone(vec_const_ct vec)
{
    return error_pass_ptr(vec_clone_f(vec, NULL, NULL, NULL));
}

vec_ct vec_clone_f(vec_const_ct vec, vec_clone_cb clone, vec_dtor_cb dtor, const void *ctx)
{
    vec_ct nvec;
    size_t i;

    assert_magic(vec);

    if(!(nvec = calloc(1, sizeof(vec_st))))
        return error_wrap_errno(calloc), NULL;

    init_magic(nvec);
    nvec->esize     = vec->esize;
    nvec->min_cap   = vec->min_cap;

    return_value_if_fail(vec->mem, nvec);

    if(!clone)
    {
        if(!vec_push_en(nvec, vec->size, vec->mem))
            return error_pass(), free(nvec), NULL;

        return nvec;
    }

    if(!vec_push_n(nvec, vec->size))
        return error_pass(), free(nvec), NULL;

    for(i = 0; i < vec->size; i++)
        if(clone(vec, nvec->mem + i * nvec->esize, ELEM(i), (void *)ctx))
            return error_pack(E_VEC_CALLBACK), vec_free_f(nvec, dtor, ctx), NULL;

    return nvec;
}

bool vec_is_empty(vec_const_ct vec)
{
    assert_magic(vec);

    return !vec->size;
}

size_t vec_size(vec_const_ct vec)
{
    assert_magic(vec);

    return vec->size;
}

size_t vec_elemsize(vec_const_ct vec)
{
    assert_magic(vec);

    return vec->esize;
}

size_t vec_capacity(vec_const_ct vec)
{
    assert_magic(vec);

    return vec->cap;
}

size_t vec_min_capacity(vec_const_ct vec)
{
    assert_magic(vec);

    return vec->min_cap;
}

size_t vec_memsize(vec_const_ct vec)
{
    return vec_memsize_f(vec, NULL, NULL);
}

size_t vec_memsize_f(vec_const_ct vec, vec_size_cb size, const void *ctx)
{
    size_t bytes, i;

    assert_magic(vec);

    bytes = sizeof(vec_st) + OFFSET + vec->cap * vec->esize;

    if(vec->mem && size)
    {
        for(i = 0; i < vec->size; i++)
            bytes += size(vec, ELEM(i), (void *)ctx);
    }

    return bytes;
}

void *vec_at(vec_const_ct vec, ssize_t pos)
{
    assert_magic(vec);

    if(pos < 0)
        pos += vec->size;

    return_error_if_fail(pos >= 0 && (size_t)pos < vec->size, E_VEC_OUT_OF_BOUNDS, NULL);

    return ELEM(pos);
}

void *vec_at_p(vec_const_ct vec, ssize_t pos)
{
    assert_magic(vec);
    assert(vec->esize == sizeof(void *));

    if(pos < 0)
        pos += vec->size;

    return_error_if_fail(pos >= 0 && (size_t)pos < vec->size, E_VEC_OUT_OF_BOUNDS, NULL);

    return *(void **)ELEM(pos);
}

void *vec_first(vec_const_ct vec)
{
    assert_magic(vec);
    return_error_if_fail(vec->size, E_VEC_EMPTY, NULL);

    return ELEM(0);
}

void *vec_first_p(vec_const_ct vec)
{
    assert_magic(vec);
    assert(vec->esize == sizeof(void *));
    return_error_if_fail(vec->size, E_VEC_EMPTY, NULL);

    return *(void **)ELEM(0);
}

void *vec_last(vec_const_ct vec)
{
    assert_magic(vec);
    return_error_if_fail(vec->size, E_VEC_EMPTY, NULL);

    return ELEM(vec->size - 1);
}

void *vec_last_p(vec_const_ct vec)
{
    assert_magic(vec);
    assert(vec->esize == sizeof(void *));
    return_error_if_fail(vec->size, E_VEC_EMPTY, NULL);

    return *(void **)ELEM(vec->size - 1);
}

ssize_t vec_pos(vec_const_ct vec, const void *elem)
{
    assert_magic(vec);
    assert(elem);

    if(vec_check_range(vec, elem))
        return error_pass(), -1;

    return ((char *)elem - vec->mem) / vec->esize;
}

int vec_get(vec_const_ct vec, void *dst, ssize_t pos)
{
    return error_pass_int(vec_get_n(vec, dst, pos, 1));
}

int vec_get_n(vec_const_ct vec, void *dst, ssize_t pos, size_t n)
{
    assert_magic(vec);

    if(pos < 0)
        pos += vec->size;

    return_error_if_fail(pos >= 0 && (size_t)pos < vec->size, E_VEC_OUT_OF_BOUNDS, -1);
    return_error_if_fail(n <= vec->size - pos, E_VEC_OUT_OF_BOUNDS, -1);

    if(dst)
        memcpy(dst, ELEM(pos), vec->esize * n);

    return 0;
}

int vec_get_first(vec_const_ct vec, void *dst)
{
    if(vec_get_n(vec, dst, 0, 1))
        return error_push(E_VEC_EMPTY), -1;

    return 0;
}

int vec_get_last(vec_const_ct vec, void *dst)
{
    if(vec_get_n(vec, dst, vec->size - 1, 1))
        return error_push(E_VEC_EMPTY), -1;

    return 0;
}

void *vec_push(vec_ct vec)
{
    return error_pass_ptr(vec_push_en(vec, 1, NULL));
}

void *vec_push_n(vec_ct vec, size_t n)
{
    return error_pass_ptr(vec_push_en(vec, n, NULL));
}

void *vec_push_e(vec_ct vec, const void *elem)
{
    return error_pass_ptr(vec_push_en(vec, 1, elem));
}

void *vec_push_p(vec_ct vec, const void *ptr)
{
    assert_magic(vec);
    assert(vec->esize == sizeof(void *));

    return error_pass_ptr(vec_push_en(vec, 1, &ptr));
}

void *vec_push_en(vec_ct vec, size_t n, const void *elems)
{
    assert_magic(vec);

    if(vec_grow(vec, n))
        return error_pass(), NULL;

    if(elems)
        memcpy(ELEM(vec->size), elems, vec->esize * n);
    else
        memset(ELEM(vec->size), 0, vec->esize * n);

    vec->size += n;

    return ELEM(vec->size - n);
}

void *vec_push_a(vec_ct vec, size_t n, ...)
{
    va_list ap;
    void *elem;

    va_start(ap, n);
    elem = error_pass_ptr(vec_push_av(vec, n, ap));
    va_end(ap);

    return elem;
}

void *vec_push_ap(vec_ct vec, size_t n, ...)
{
    va_list ap;
    void *elem;

    va_start(ap, n);
    elem = error_pass_ptr(vec_push_apv(vec, n, ap));
    va_end(ap);

    return elem;
}

void *vec_push_av(vec_ct vec, size_t n, va_list ap)
{
    size_t i;

    assert_magic(vec);

    if(vec_grow(vec, n))
        return error_pass(), NULL;

    for(i = 0; i < n; i++)
        memcpy(ELEM(vec->size + i), va_arg(ap, void *), vec->esize);

    vec->size += n;

    return ELEM(vec->size - n);
}

void *vec_push_apv(vec_ct vec, size_t n, va_list ap)
{
    size_t i;

    assert_magic(vec);
    assert(vec->esize == sizeof(void *));

    if(vec_grow(vec, n))
        return error_pass(), NULL;

    for(i = 0; i < n; i++)
        *(void **)ELEM(vec->size + i) = va_arg(ap, void *);

    vec->size += n;

    return ELEM(vec->size - n);
}

/// Insert n element(s).
///
/// \param vec      vector
/// \param pos      insert position, negative value counts from last element
/// \param n        number of elements to insert
/// \param elems    element(s) to insert, may be NULL
///
/// \returns                            pointer to first new element
/// \retval NULL/E_VEC_OUT_OF_BOUNDS    \p pos is out of bounds
/// \retval NULL/E_GENERIC_OOM          out of memory
static void *vec_insert_generic(vec_ct vec, ssize_t pos, size_t n, const void *elems)
{
    size_t size;

    if(pos < 0)
        pos += vec->size;

    return_error_if_fail(pos >= 0 && (size_t)pos <= vec->size, E_VEC_OUT_OF_BOUNDS, NULL);

    if(!n)
        return ELEM(pos);

    if(vec_grow(vec, n))
        return error_pass(), NULL;

    if((size = vec->size - pos))
        memmove(ELEM(pos + n), ELEM(pos), size * vec->esize);

    if(elems)
        memcpy(ELEM(pos), elems, n * vec->esize);
    else
        memset(ELEM(pos), 0, n * vec->esize);

    vec->size += n;

    return ELEM(pos);
}

void *vec_insert(vec_ct vec, ssize_t pos)
{
    assert_magic(vec);

    return error_pass_ptr(vec_insert_generic(vec, pos, 1, NULL));
}

void *vec_insert_e(vec_ct vec, ssize_t pos, const void *elem)
{
    assert_magic(vec);

    return error_pass_ptr(vec_insert_generic(vec, pos, 1, elem));
}

void *vec_insert_p(vec_ct vec, ssize_t pos, const void *ptr)
{
    assert_magic(vec);
    assert(vec->esize == sizeof(void *));

    return error_pass_ptr(vec_insert_generic(vec, pos, 1, &ptr));
}

void *vec_insert_n(vec_ct vec, ssize_t pos, size_t n)
{
    assert_magic(vec);

    return error_pass_ptr(vec_insert_generic(vec, pos, n, NULL));
}

void *vec_insert_en(vec_ct vec, ssize_t pos, size_t n, const void *elems)
{
    assert_magic(vec);

    return error_pass_ptr(vec_insert_generic(vec, pos, n, elems));
}

void *vec_insert_before(vec_ct vec, const void *dst)
{
    assert_magic(vec);
    assert(dst);

    if(vec_check_range(vec, dst))
        return error_pass(), NULL;

    return error_pass_ptr(vec_insert_generic(vec, POS(dst), 1, NULL));
}

void *vec_insert_before_e(vec_ct vec, const void *dst, const void *elem)
{
    assert_magic(vec);
    assert(dst);

    if(vec_check_range(vec, dst))
        return error_pass(), NULL;

    return error_pass_ptr(vec_insert_generic(vec, POS(dst), 1, elem));
}

void *vec_insert_before_p(vec_ct vec, const void *dst, const void *ptr)
{
    assert_magic(vec);
    assert(dst);
    assert(vec->esize == sizeof(void *));

    if(vec_check_range(vec, dst))
        return error_pass(), NULL;

    return error_pass_ptr(vec_insert_generic(vec, POS(dst), 1, &ptr));
}

void *vec_insert_before_n(vec_ct vec, const void *dst, size_t n)
{
    assert_magic(vec);
    assert(dst);

    if(vec_check_range(vec, dst))
        return error_pass(), NULL;

    return error_pass_ptr(vec_insert_generic(vec, POS(dst), n, NULL));
}

void *vec_insert_before_en(vec_ct vec, const void *dst, size_t n, const void *elems)
{
    assert_magic(vec);
    assert(dst);

    if(vec_check_range(vec, dst))
        return error_pass(), NULL;

    return error_pass_ptr(vec_insert_generic(vec, POS(dst), n, elems));
}

void *vec_insert_after(vec_ct vec, const void *dst)
{
    assert_magic(vec);
    assert(dst);

    if(vec_check_range(vec, dst))
        return error_pass(), NULL;

    return error_pass_ptr(vec_insert_generic(vec, POS(dst) + 1, 1, NULL));
}

void *vec_insert_after_e(vec_ct vec, const void *dst, const void *elem)
{
    assert_magic(vec);
    assert(dst);

    if(vec_check_range(vec, dst))
        return error_pass(), NULL;

    return error_pass_ptr(vec_insert_generic(vec, POS(dst) + 1, 1, elem));
}

void *vec_insert_after_p(vec_ct vec, const void *dst, const void *ptr)
{
    assert_magic(vec);
    assert(dst);
    assert(vec->esize == sizeof(void *));

    if(vec_check_range(vec, dst))
        return error_pass(), NULL;

    return error_pass_ptr(vec_insert_generic(vec, POS(dst) + 1, 1, &ptr));
}

void *vec_insert_after_n(vec_ct vec, const void *dst, size_t n)
{
    assert_magic(vec);
    assert(dst);

    if(vec_check_range(vec, dst))
        return error_pass(), NULL;

    return error_pass_ptr(vec_insert_generic(vec, POS(dst) + 1, n, NULL));
}

void *vec_insert_after_en(vec_ct vec, const void *dst, size_t n, const void *elems)
{
    assert_magic(vec);
    assert(dst);

    if(vec_check_range(vec, dst))
        return error_pass(), NULL;

    return error_pass_ptr(vec_insert_generic(vec, POS(dst) + 1, n, elems));
}

int vec_set(vec_ct vec, ssize_t pos, const void *elem)
{
    return error_pass_int(vec_set_f(vec, pos, elem, NULL, NULL));
}

int vec_set_f(vec_ct vec, ssize_t pos, const void *elem, vec_dtor_cb dtor, const void *ctx)
{
    assert_magic(vec);

    if(pos < 0)
        pos += vec->size;

    return_error_if_fail(pos >= 0 && (size_t)pos <= vec->size, E_VEC_OUT_OF_BOUNDS, -1);

    if(dtor)
        dtor(vec, ELEM(pos), (void *)ctx);

    if(elem)
        memcpy(ELEM(pos), elem, vec->esize);
    else
        memset(ELEM(pos), 0, vec->esize);

    return 0;
}

/// Remove last n element(s).
///
/// \param vec          vector
/// \param[out] dst     element(s) to fill, may be NULL
/// \param n            number of elements to remove
/// \param dtor         callback to destroy element(s), may be NULL
/// \param ctx          \p dtor context
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  not enough elements to remove
static int vec_pop_generic(vec_ct vec, void *dst, size_t n, vec_dtor_cb dtor, const void *ctx)
{
    size_t e;

    return_error_if_fail(n <= vec->size, E_VEC_OUT_OF_BOUNDS, -1);

    vec->size -= n;

    if(dst)
        memcpy(dst, ELEM(vec->size), vec->esize * n);

    if(dtor)
    {
        for(e = 0; e < n; e++)
            dtor(vec, ELEM(vec->size + e), (void *)ctx);
    }

    vec_shrink(vec);

    return 0;
}

int vec_pop(vec_ct vec)
{
    assert_magic(vec);

    if(vec_pop_generic(vec, NULL, 1, NULL, NULL))
        return error_push(E_VEC_EMPTY), -1;

    return 0;
}

int vec_pop_e(vec_ct vec, void *dst)
{
    assert_magic(vec);

    if(vec_pop_generic(vec, dst, 1, NULL, NULL))
        return error_push(E_VEC_EMPTY), -1;

    return 0;
}

void *vec_pop_p(vec_ct vec)
{
    void *p;

    assert_magic(vec);
    assert(vec->esize == sizeof(void *));

    if(vec_pop_generic(vec, &p, 1, NULL, NULL))
        return error_push(E_VEC_EMPTY), NULL;

    return p;
}

int vec_pop_f(vec_ct vec, vec_dtor_cb dtor, const void *ctx)
{
    assert_magic(vec);

    if(vec_pop_generic(vec, NULL, 1, dtor, ctx))
        return error_push(E_VEC_EMPTY), -1;

    return 0;
}

int vec_pop_n(vec_ct vec, size_t n)
{
    assert_magic(vec);

    return error_pass_int(vec_pop_generic(vec, NULL, n, NULL, NULL));
}

int vec_pop_en(vec_ct vec, void *dst, size_t n)
{
    assert_magic(vec);

    return error_pass_int(vec_pop_generic(vec, dst, n, NULL, NULL));
}

int vec_pop_fn(vec_ct vec, size_t n, vec_dtor_cb dtor, const void *ctx)
{
    assert_magic(vec);

    return error_pass_int(vec_pop_generic(vec, NULL, n, dtor, ctx));
}

/// Remove n element(s) at position.
///
/// \param      vec     vector
/// \param[out] dst     if not NULL, fill with removed element(s)
/// \param      pos     position of element(s), negative value counts from last element
/// \param      n       number of elements to remove
/// \param      dtor    if not NULL, apply onto each removed element
/// \param      ctx     \p dtor context
///
/// \retval 0                       success
/// \retval -1/E_VEC_OUT_OF_BOUNDS  \p pos or \p pos + \p n is out of bounds
static int vec_remove_generic(vec_ct vec, void *dst, ssize_t pos, size_t n, vec_dtor_cb dtor, const void *ctx)
{
    size_t e;

    if(pos < 0)
        pos += vec->size;

    return_error_if_fail(pos >= 0 && (size_t)pos < vec->size, E_VEC_OUT_OF_BOUNDS, -1);
    return_error_if_fail(n <= vec->size - pos, E_VEC_OUT_OF_BOUNDS, -1);

    if(!n)
        return 0;

    if(dst)
        memcpy(dst, ELEM(pos), vec->esize * n);

    if(dtor)
    {
        for(e = 0; e < n; e++)
            dtor(vec, ELEM(pos + e), (void *)ctx);
    }

    memmove(ELEM(pos), ELEM(pos + n), (vec->size - pos - n) * vec->esize);

    vec->size -= n;
    vec_shrink(vec);

    return 0;
}

int vec_remove(vec_ct vec, void *elem)
{
    assert_magic(vec);
    assert(elem);

    if(vec_check_range(vec, elem))
        return error_pass(), -1;

    return error_pass_int(vec_remove_generic(vec, NULL, POS(elem), 1, NULL, NULL));
}

int vec_remove_f(vec_ct vec, void *elem, vec_dtor_cb dtor, const void *ctx)
{
    assert_magic(vec);
    assert(elem);

    if(vec_check_range(vec, elem))
        return error_pass(), -1;

    return error_pass_int(vec_remove_generic(vec, NULL, POS(elem), 1, dtor, ctx));
}

int vec_remove_n(vec_ct vec, void *elem, size_t n)
{
    assert_magic(vec);
    assert(elem);

    if(vec_check_range(vec, elem))
        return error_pass(), -1;

    return error_pass_int(vec_remove_generic(vec, NULL, POS(elem), n, NULL, NULL));
}

int vec_remove_fn(vec_ct vec, void *elem, size_t n, vec_dtor_cb dtor, const void *ctx)
{
    assert_magic(vec);
    assert(elem);

    if(vec_check_range(vec, elem))
        return error_pass(), -1;

    return error_pass_int(vec_remove_generic(vec, NULL, POS(elem), n, dtor, ctx));
}

int vec_remove_at(vec_ct vec, ssize_t pos)
{
    assert_magic(vec);

    return error_pass_int(vec_remove_generic(vec, NULL, pos, 1, NULL, NULL));
}

int vec_remove_at_e(vec_ct vec, void *dst, ssize_t pos)
{
    assert_magic(vec);

    return error_pass_int(vec_remove_generic(vec, dst, pos, 1, NULL, NULL));
}

void *vec_remove_at_p(vec_ct vec, ssize_t pos)
{
    void *p;

    assert_magic(vec);
    assert(vec->esize == sizeof(void *));

    if(vec_remove_generic(vec, &p, pos, 1, NULL, NULL))
        return error_pass(), NULL;

    return p;
}

int vec_remove_at_f(vec_ct vec, ssize_t pos, vec_dtor_cb dtor, const void *ctx)
{
    assert_magic(vec);

    return error_pass_int(vec_remove_generic(vec, NULL, pos, 1, dtor, ctx));
}

int vec_remove_at_n(vec_ct vec, ssize_t pos, size_t n)
{
    assert_magic(vec);

    return error_pass_int(vec_remove_generic(vec, NULL, pos, n, NULL, NULL));
}

int vec_remove_at_en(vec_ct vec, void *dst, ssize_t pos, size_t n)
{
    assert_magic(vec);

    return error_pass_int(vec_remove_generic(vec, dst, pos, n, NULL, NULL));
}

int vec_remove_at_fn(vec_ct vec, ssize_t pos, size_t n, vec_dtor_cb dtor, const void *ctx)
{
    assert_magic(vec);

    return error_pass_int(vec_remove_generic(vec, NULL, pos, n, dtor, ctx));
}

void *vec_find(vec_const_ct vec, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;

    if((pos = vec_find_pos(vec, pred, ctx)) < 0)
        return error_pass(), NULL;

    return ELEM(pos);
}

int vec_find_e(vec_const_ct vec, void *dst, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;

    if((pos = vec_find_pos(vec, pred, ctx)) < 0)
        return error_pass(), -1;

    if(dst)
        memcpy(dst, ELEM(pos), vec->esize);

    return 0;
}

void *vec_find_p(vec_const_ct vec, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;

    assert_magic(vec);
    assert(vec->esize == sizeof(void *));

    if((pos = vec_find_pos(vec, pred, ctx)) < 0)
        return error_pass(), NULL;

    return *(void **)ELEM(pos);
}

void *vec_find_r(vec_const_ct vec, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;

    if((pos = vec_find_pos_r(vec, pred, ctx)) < 0)
        return error_pass(), NULL;

    return ELEM(pos);
}

int vec_find_re(vec_const_ct vec, void *dst, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;

    if((pos = vec_find_pos_r(vec, pred, ctx)) < 0)
        return error_pass(), -1;

    if(dst)
        memcpy(dst, ELEM(pos), vec->esize);

    return 0;
}

void *vec_find_rp(vec_const_ct vec, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;

    assert_magic(vec);
    assert(vec->esize == sizeof(void *));

    if((pos = vec_find_pos_r(vec, pred, ctx)) < 0)
        return error_pass(), NULL;

    return *(void **)ELEM(pos);
}

ssize_t vec_find_pos(vec_const_ct vec, vec_pred_cb pred, const void *ctx)
{
    size_t i;

    assert_magic(vec);
    assert(pred);

    for(i = 0; i < vec->size; i++)
        if(pred(vec, ELEM(i), (void *)ctx))
            return i;

    return_error_if_reached(E_VEC_NOT_FOUND, -1);
}

ssize_t vec_find_pos_r(vec_const_ct vec, vec_pred_cb pred, const void *ctx)
{
    size_t i;

    assert_magic(vec);
    assert(pred);

    for(i = vec->size; i > 0; i--)
        if(pred(vec, ELEM(i - 1), (void *)ctx))
            return i - 1;

    return_error_if_reached(E_VEC_NOT_FOUND, -1);
}

int vec_find_remove(vec_ct vec, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;

    if((pos = vec_find_pos(vec, pred, ctx)) < 0)
        return error_pass(), -1;

    return vec_remove_generic(vec, NULL, pos, 1, NULL, NULL);
}

int vec_find_remove_e(vec_ct vec, void *dst, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;

    if((pos = vec_find_pos(vec, pred, ctx)) < 0)
        return error_pass(), -1;

    if(dst)
        memcpy(dst, ELEM(pos), vec->esize);

    return vec_remove_generic(vec, NULL, pos, 1, NULL, NULL);
}

void *vec_find_remove_p(vec_ct vec, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;
    void *p;

    assert_magic(vec);
    assert(vec->esize == sizeof(void *));

    if((pos = vec_find_pos(vec, pred, ctx)) < 0)
        return error_pass(), NULL;

    vec_remove_generic(vec, &p, pos, 1, NULL, NULL);

    return p;
}

int vec_find_remove_f(vec_ct vec, vec_pred_cb pred, const void *pred_ctx, vec_dtor_cb dtor, const void *dtor_ctx)
{
    ssize_t pos;

    if((pos = vec_find_pos(vec, pred, pred_ctx)) < 0)
        return error_pass(), -1;

    return vec_remove_generic(vec, NULL, pos, 1, dtor, dtor_ctx);
}

int vec_find_remove_r(vec_ct vec, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;

    if((pos = vec_find_pos_r(vec, pred, ctx)) < 0)
        return error_pass(), -1;

    return vec_remove_generic(vec, NULL, pos, 1, NULL, NULL);
}

int vec_find_remove_re(vec_ct vec, void *dst, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;

    if((pos = vec_find_pos_r(vec, pred, ctx)) < 0)
        return error_pass(), -1;

    if(dst)
        memcpy(dst, ELEM(pos), vec->esize);

    return vec_remove_generic(vec, NULL, pos, 1, NULL, NULL);
}

void *vec_find_remove_rp(vec_ct vec, vec_pred_cb pred, const void *ctx)
{
    ssize_t pos;
    void *p;

    assert_magic(vec);
    assert(vec->esize == sizeof(void *));

    if((pos = vec_find_pos_r(vec, pred, ctx)) < 0)
        return error_pass(), NULL;

    vec_remove_generic(vec, &p, pos, 1, NULL, NULL);

    return p;
}

int vec_find_remove_rf(vec_ct vec, vec_pred_cb pred, const void *pred_ctx, vec_dtor_cb dtor, const void *dtor_ctx)
{
    ssize_t pos;

    if((pos = vec_find_pos_r(vec, pred, pred_ctx)) < 0)
        return error_pass(), -1;

    return vec_remove_generic(vec, NULL, pos, 1, dtor, dtor_ctx);
}

size_t vec_find_remove_all(vec_ct vec, vec_pred_cb pred, const void *ctx)
{
    return vec_find_remove_all_f(vec, pred, ctx, NULL, NULL);
}

size_t vec_find_remove_all_f(vec_ct vec, vec_pred_cb pred, const void *pred_ctx, vec_dtor_cb dtor, const void *dtor_ctx)
{
    size_t i, keep, size;

    assert_magic(vec);
    assert(pred);

    for(i = 0, keep = 0, size = 0; i < vec->size; i++)
        if(pred(vec, ELEM(i), (void *)pred_ctx))
        {
            if(dtor)
                dtor(vec, ELEM(i), (void *)dtor_ctx);

            if(keep && i > keep)
                memmove(ELEM(size), ELEM(i - keep), keep);

            size    += keep;
            keep    = 0;
        }
        else
        {
            keep++;
        }

    if(keep && i > keep)
        memmove(ELEM(size), ELEM(i - keep), keep);

    size += keep;

    if(!(i = vec->size - size))
        return 0;

    vec->size = size;

    return i;
}

int vec_swap(vec_ct vec, ssize_t pos1, ssize_t pos2)
{
    void *elem;

    assert_magic(vec);

    if(pos1 < 0)
        pos1 += vec->size;

    if(pos2 < 0)
        pos2 += vec->size;

    return_error_if_fail(pos1 >= 0 && (size_t)pos1 < vec->size, E_VEC_OUT_OF_BOUNDS, -1);
    return_error_if_fail(pos2 >= 0 && (size_t)pos2 < vec->size, E_VEC_OUT_OF_BOUNDS, -1);

    if(pos1 == pos2)
        return 0;

    elem = alloca(vec->esize);
    memcpy(elem, ELEM(pos1), vec->esize);
    memcpy(ELEM(pos1), ELEM(pos2), vec->esize);
    memcpy(ELEM(pos2), elem, vec->esize);

    return 0;
}

int vec_get_buffer(vec_ct vec, void **buf, size_t *size, size_t *capacity)
{
    assert_magic(vec);
    assert(buf);

    return_error_if_fail(vec->mem, E_VEC_NO_BUFFER, -1);

    if(size)
        *size = vec->size;

    if(capacity)
        *capacity = vec->cap + (OFFSET ? 1 : 0);

    if(OFFSET)
        memmove(vec->mem - OFFSET, vec->mem, vec->size * vec->esize);

    *buf = vec->mem - OFFSET;

    vec_set_buf(vec, NULL, 0, 0);

    return 0;
}

size_t vec_truncate(vec_ct vec, size_t size)
{
    return vec_truncate_f(vec, size, NULL, NULL);
}

size_t vec_truncate_f(vec_ct vec, size_t size, vec_dtor_cb dtor, const void *ctx)
{
    size_t n;

    assert_magic(vec);

    if(size >= vec->size)
        return 0;

    if(dtor)
    {
        for(n = size; n < vec->size; n++)
            dtor(vec, ELEM(n), (void *)ctx);
    }

    n           = vec->size - size;
    vec->size   = size;

    vec_shrink(vec);

    return n;
}

int vec_set_capacity(vec_ct vec, size_t capacity)
{
    return error_pass_int(vec_set_capacity_f(vec, capacity, NULL, NULL));
}

int vec_set_capacity_f(vec_ct vec, size_t capacity, vec_dtor_cb dtor, const void *ctx)
{
    size_t i;

    assert_magic(vec);

    capacity = MAX(capacity, vec->min_cap);

    if(capacity < vec->size)
    {
        if(dtor)
        {
            for(i = capacity; i < vec->size; i++)
                dtor(vec, ELEM(i), (void *)ctx);
        }

        vec->size = capacity;
    }

    if(vec_resize(vec, capacity))
        return error_pass(), -1;

    return 0;
}

int vec_fold(vec_const_ct vec, vec_fold_cb fold, const void *ctx)
{
    size_t i;
    int rc;

    assert_magic(vec);
    assert(fold);

    for(i = 0; i < vec->size; i++)
        if((rc = fold(vec, i, ELEM(i), (void *)ctx)))
            return error_pack_int(E_VEC_CALLBACK, rc);

    return 0;
}

int vec_fold_r(vec_const_ct vec, vec_fold_cb fold, const void *ctx)
{
    size_t i;
    int rc;

    assert_magic(vec);
    assert(fold);

    for(i = 0; i < vec->size; i++)
        if((rc = fold(vec, i, ELEM(vec->size - i - 1), (void *)ctx)))
            return error_pack_int(E_VEC_CALLBACK, rc);

    return 0;
}

void vec_sort(vec_ct vec, vec_sort_cb sort, const void *ctx)
{
    assert_magic(vec);
    assert(sort);

    if(vec->mem)
        qsort_r(vec->mem, vec->size, vec->esize, sort, (void *)ctx);
}
