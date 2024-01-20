/*
 * Copyright (c) 2019-2024 Martin Rödel a.k.a. Yomin Nimoy
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

#include "cont.h"
#include <ytil/test/run.h>
#include <ytil/test/test.h>
#include <ytil/con/ring.h>

static const struct not_a_ring
{
    int foo;
} not_a_ring = { 123 };

static const int i[] = { 1, 2, 3, 4, 5 };
static const int *pi[] = { &i[0], &i[1], &i[2], &i[3], &i[4] };
static int count, *j, k, a, b;
static ring_ct ring, ring2;


TEST_SETUP(ring_new_empty)
{
    test_ptr_success(ring = ring_new_c(4, sizeof(int)));
}

TEST_SETUP(ring_new)
{
    test_ptr_success(ring = ring_new_c(4, sizeof(int)));
    test_ptr_success(ring_put_e(ring, &i[0]));
    test_ptr_success(ring_put_e(ring, &i[1]));
    test_ptr_success(ring_put_e(ring, &i[2]));
    test_ptr_success(ring_put_e(ring, &i[3]));
}

TEST_SETUP(ring_new_ptr_empty)
{
    test_ptr_success(ring = ring_new_c(4, sizeof(int *)));
}

TEST_SETUP(ring_new_ptr)
{
    test_ptr_success(ring = ring_new_c(4, sizeof(int *)));
    test_ptr_success(ring_put_p(ring, pi[0]));
    test_ptr_success(ring_put_p(ring, pi[1]));
    test_ptr_success(ring_put_p(ring, pi[2]));
    test_ptr_success(ring_put_p(ring, pi[3]));
}

TEST_TEARDOWN(ring_free)
{
    test_void(ring_free(ring));
}

TEST_TEARDOWN(ring2_free)
{
    test_void(ring_free(ring));
    test_void(ring_free(ring2));
}

TEST_CASE_ABORT(ring_new__invalid_elemsize)
{
    test_void(ring_new(0));
}

TEST_CASE_ABORT(ring_new_c__invalid_elemsize)
{
    test_void(ring_new_c(1, 0));
}

TEST_CASE_ABORT(ring_free__invalid_magic)
{
    ring_free((ring_ct)&not_a_ring);
}

static void test_ring_dtor(ring_const_ct r, void *elem, void *ctx)
{
    int *count = ctx;

    count[0]++;
}

TEST_CASE_ABORT(ring_free_f__invalid_magic)
{
    ring_free_f((ring_ct)&not_a_ring, test_ring_dtor, &count);
}

TEST_CASE_FIX(ring_free_f, ring_new, no_teardown)
{
    count = 0;
    test_void(ring_free_f(ring, test_ring_dtor, &count));
    test_int_eq(count, 4);
}

TEST_CASE_ABORT(ring_free_if_empty__invalid_magic)
{
    ring_free_if_empty((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_free_if_empty__empty, ring_new_empty, no_teardown)
{
    test_ptr_eq(NULL, ring_free_if_empty(ring));
}

TEST_CASE_FIX(ring_free_if_empty, ring_new, no_teardown)
{
    test_ptr_ne(NULL, ring_free_if_empty(ring));
}

TEST_CASE_ABORT(ring_free_if_empty_f__invalid_magic)
{
    ring_free_if_empty_f((ring_ct)&not_a_ring, test_ring_dtor, &count);
}

TEST_CASE_FIX(ring_free_if_empty_f__empty, ring_new_empty, no_teardown)
{
    count = 0;
    test_ptr_eq(NULL, ring_free_if_empty_f(ring, test_ring_dtor, &count));
    test_int_eq(count, 0);
}

TEST_CASE_FIX(ring_free_if_empty_f, ring_new, no_teardown)
{
    count = 0;
    test_ptr_ne(NULL, ring_free_if_empty_f(ring, test_ring_dtor, &count));
    test_int_eq(count, 0);
}

TEST_CASE_ABORT(ring_clear__invalid_magic)
{
    ring_clear((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_clear, ring_new, ring_free)
{
    test_void(ring_clear(ring));
    test_true(ring_is_empty(ring));
}

TEST_CASE_ABORT(ring_clear_f__invalid_magic)
{
    ring_clear_f((ring_ct)&not_a_ring, test_ring_dtor, &count);
}

TEST_CASE_FIX(ring_clear_f, ring_new, ring_free)
{
    count = 0;
    test_void(ring_clear_f(ring, test_ring_dtor, &count));
    test_true(ring_is_empty(ring));
    test_int_eq(count, 4);
}

TEST_CASE_ABORT(ring_clone__invalid_magic)
{
    ring_clone((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_clone__empty, ring_new_empty, ring2_free)
{
    test_ptr_success(ring2 = ring_clone(ring));
    test_ptr_ne(ring, ring2);
    test_uint_eq(ring_size(ring2), 0);
}

TEST_CASE_FIX(ring_clone, ring_new, ring2_free)
{
    test_ptr_success(ring2 = ring_clone(ring));
    test_ptr_ne(ring, ring2);
    test_uint_eq(ring_size(ring), ring_size(ring2));

    while(!ring_is_empty(ring))
    {
        test_int_success(ring_get(ring, &a));
        test_int_success(ring_get(ring2, &b));
        test_int_eq(a, b);
    }
}

TEST_CASE_ABORT(ring_clone_f__invalid_magic)
{
    ring_clone_f((ring_ct)&not_a_ring, NULL, NULL, NULL);
}

TEST_CASE_FIX(ring_clone_f__empty, ring_new_empty, ring2_free)
{
    test_ptr_success(ring2 = ring_clone_f(ring, NULL, NULL, NULL));
    test_ptr_ne(ring, ring2);
    test_uint_eq(ring_size(ring2), 0);
}

TEST_CASE_FIX(ring_clone_f__shallow, ring_new, ring2_free)
{
    test_ptr_success(ring2 = ring_clone_f(ring, NULL, NULL, NULL));
    test_ptr_ne(ring, ring2);
    test_uint_eq(ring_size(ring), ring_size(ring2));

    while(!ring_is_empty(ring))
    {
        test_int_success(ring_get(ring, &a));
        test_int_success(ring_get(ring2, &b));
        test_int_eq(a, b);
    }
}

static int test_ring_clone(ring_const_ct r, void *dst, const void *src, void *ctx)
{
    const int *isrc = src;
    int *idst = dst;

    *idst = *isrc + 10;

    return 0;
}

TEST_CASE_FIX(ring_clone_f__deep, ring_new, ring2_free)
{
    count = 0;
    test_ptr_success(ring2 = ring_clone_f(ring, test_ring_clone, test_ring_dtor, &count));
    test_ptr_ne(ring, ring2);
    test_uint_eq(ring_size(ring), ring_size(ring2));
    test_int_eq(count, 0);

    while(!ring_is_empty(ring))
    {
        test_int_success(ring_get(ring, &a));
        test_int_success(ring_get(ring2, &b));
        test_int_eq(a + 10, b);
    }
}

static int test_ring_clone_fail(ring_const_ct r, void *dst, const void *src, void *ctx)
{
    int *count = ctx;

    if(*count == 20)
        return error_set_s(GENERIC, E_GENERIC_OOM), -1;

    *count += 10;

    return 0;
}

TEST_CASE_FIX(ring_clone_f__deep_fail, ring_new, ring_free)
{
    count = 0;
    test_ptr_error(ring_clone_f(ring, test_ring_clone_fail, test_ring_dtor, &count), E_GENERIC_OOM);
    test_int_eq(count, 24);
}

TEST_CASE_ABORT(ring_is_empty__invalid_magic)
{
    ring_is_empty((ring_ct)&not_a_ring);
}

TEST_CASE(ring_is_empty__null)
{
    test_true(ring_is_empty(NULL));
}

TEST_CASE_FIX(ring_is_empty, ring_new_empty, ring_free)
{
    test_true(ring_is_empty(ring));
    test_ptr_success(ring_put(ring));
    test_false(ring_is_empty(ring));
    test_int_success(ring_get(ring, NULL));
    test_true(ring_is_empty(ring));
}

TEST_CASE_ABORT(ring_size__invalid_magic)
{
    ring_size((ring_ct)&not_a_ring);
}

TEST_CASE(ring_size__null)
{
    test_uint_eq(ring_size(NULL), 0);
}

TEST_CASE_FIX(ring_size, ring_new_empty, ring_free)
{
    test_uint_eq(ring_size(ring), 0);
    test_ptr_success(ring_put(ring));
    test_uint_eq(ring_size(ring), 1);
    test_int_success(ring_get(ring, NULL));
    test_uint_eq(ring_size(ring), 0);
}

TEST_CASE_ABORT(ring_elemsize__invalid_magic)
{
    ring_elemsize((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_elemsize, ring_new_empty, ring_free)
{
    test_uint_eq(ring_elemsize(ring), sizeof(int));
}

TEST_CASE_ABORT(ring_capacity__invalid_magic)
{
    ring_capacity((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_capacity, ring_new_empty, ring_free)
{
    test_uint_eq(ring_capacity(ring), 4);
}

TEST_CASE_ABORT(ring_memsize__invalid_magic)
{
    ring_memsize((ring_ct)&not_a_ring);
}

TEST_CASE_ABORT(ring_memsize_f__invalid_magic)
{
    ring_memsize_f((ring_ct)&not_a_ring, NULL, NULL);
}

TEST_CASE_ABORT(ring_put__invalid_magic)
{
    ring_put((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_put, ring_new_empty, ring_free)
{
    test_ptr_success(ring_put(ring));
    test_uint_eq(ring_size(ring), 1);
}

TEST_CASE_FIX(ring_put__overflow, ring_new, ring_free)
{
    test_uint_eq(ring_size(ring), 4);
    test_ptr_error(ring_put(ring), E_RING_FULL);
    test_uint_eq(ring_size(ring), 4);
}

TEST_CASE_ABORT(ring_put_e__invalid_magic)
{
    ring_put_e((ring_ct)&not_a_ring, &i[4]);
}

TEST_CASE_FIX(ring_put_e, ring_new_empty, ring_free)
{
    test_ptr_success(j = ring_put_e(ring, &i[4]));
    test_uint_eq(ring_size(ring), 1);
    test_ptr_eq(j, ring_peek(ring));
    test_int_eq(*j, i[4]);
}

TEST_CASE_FIX(ring_put_e__overflow, ring_new, ring_free)
{
    test_uint_eq(ring_size(ring), 4);
    test_ptr_error(ring_put_e(ring, &i[4]), E_RING_FULL);
    test_uint_eq(ring_size(ring), 4);
}

TEST_CASE_ABORT(ring_put_p__invalid_magic)
{
    ring_put_p((ring_ct)&not_a_ring, &i[4]);
}

TEST_CASE_FIX_ABORT(ring_put_p__invalid_type, ring_new, ring_free)
{
    ring_put_p(ring, pi[4]);
}

TEST_CASE_FIX(ring_put_p, ring_new_ptr_empty, ring_free)
{
    test_ptr_success(j = ring_put_p(ring, pi[4]));
    test_uint_eq(ring_size(ring), 1);
    test_ptr_eq(j, ring_peek(ring));
    test_ptr_eq(*(int **)j, pi[4]);
}

TEST_CASE_FIX(ring_put_p__overflow, ring_new_ptr, ring_free)
{
    test_uint_eq(ring_size(ring), 4);
    test_ptr_error(ring_put_p(ring, pi[4]), E_RING_FULL);
    test_uint_eq(ring_size(ring), 4);
}

TEST_CASE_ABORT(ring_put_overwrite__invalid_magic)
{
    ring_put_overwrite((ring_ct)&not_a_ring, NULL, NULL);
}

TEST_CASE_FIX(ring_put_overwrite, ring_new_empty, ring_free)
{
    count = 0;
    test_ptr_success(ring_put_overwrite(ring, test_ring_dtor, &count));
    test_uint_eq(ring_size(ring), 1);
    test_int_eq(count, 0);
}

TEST_CASE_FIX(ring_put_overwrite__overflow, ring_new, ring_free)
{
    count = 0;
    test_uint_eq(ring_size(ring), 4);
    test_ptr_success(j = ring_put_overwrite(ring, test_ring_dtor, &count));
    test_uint_eq(ring_size(ring), 4);
    test_int_eq(count, 1);
    test_ptr_eq(j, ring_peek_head(ring));
    test_int_eq(i[1], *(int *)ring_peek(ring));
}

TEST_CASE_ABORT(ring_put_overwrite_e__invalid_magic)
{
    ring_put_overwrite_e((ring_ct)&not_a_ring, &i[4], NULL, NULL);
}

TEST_CASE_FIX(ring_put_overwrite_e, ring_new_empty, ring_free)
{
    count = 0;
    test_ptr_success(j = ring_put_overwrite_e(ring, &i[4], test_ring_dtor, &count));
    test_uint_eq(ring_size(ring), 1);
    test_int_eq(count, 0);
    test_ptr_eq(j, ring_peek(ring));
    test_int_eq(*j, i[4]);
}

TEST_CASE_FIX(ring_put_overwrite_e__overflow, ring_new, ring_free)
{
    count = 0;
    test_uint_eq(ring_size(ring), 4);
    test_ptr_success(j = ring_put_overwrite_e(ring, &i[4], test_ring_dtor, &count));
    test_uint_eq(ring_size(ring), 4);
    test_int_eq(count, 1);
    test_ptr_eq(j, ring_peek_head(ring));
    test_int_eq(*j, i[4]);
    test_int_eq(i[1], *(int *)ring_peek(ring));
}

TEST_CASE_ABORT(ring_put_overwrite_p__invalid_magic)
{
    ring_put_overwrite_p((ring_ct)&not_a_ring, pi[4], NULL, NULL);
}

TEST_CASE_FIX_ABORT(ring_put_overwrite_p__invalid_type, ring_new, ring_free)
{
    ring_put_overwrite_p(ring, pi[4], NULL, NULL);
}

TEST_CASE_FIX(ring_put_overwrite_p, ring_new_ptr_empty, ring_free)
{
    count = 0;
    test_ptr_success(j = ring_put_overwrite_p(ring, pi[4], test_ring_dtor, &count));
    test_uint_eq(ring_size(ring), 1);
    test_int_eq(count, 0);
    test_ptr_eq(j, ring_peek(ring));
    test_ptr_eq(*(int **)j, pi[4]);
}

TEST_CASE_FIX(ring_put_overwrite_p__overflow, ring_new_ptr, ring_free)
{
    count = 0;
    test_uint_eq(ring_size(ring), 4);
    test_ptr_success(j = ring_put_overwrite_p(ring, pi[4], test_ring_dtor, &count));
    test_uint_eq(ring_size(ring), 4);
    test_int_eq(count, 1);
    test_ptr_eq(j, ring_peek_head(ring));
    test_ptr_eq(*(int **)j, pi[4]);
    test_ptr_eq(pi[1], *(int **)ring_peek(ring));
}

TEST_CASE_ABORT(ring_peek__invalid_magic)
{
    ring_peek((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_peek__empty, ring_new_empty, ring_free)
{
    test_ptr_error(ring_peek(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_peek, ring_new, ring_free)
{
    test_ptr_success(j = ring_peek(ring));
    test_uint_eq(ring_size(ring), 4);
    test_int_eq(*j, i[0]);
}

TEST_CASE_ABORT(ring_peek_p__invalid_magic)
{
    ring_peek_p((ring_ct)&not_a_ring);
}

TEST_CASE_FIX_ABORT(ring_peek_p__invalid_type, ring_new, ring_free)
{
    ring_peek_p(ring);
}

TEST_CASE_FIX(ring_peek_p__empty, ring_new_ptr_empty, ring_free)
{
    test_ptr_error(ring_peek_p(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_peek_p, ring_new_ptr, ring_free)
{
    test_ptr_success(j = ring_peek_p(ring));
    test_uint_eq(ring_size(ring), 4);
    test_ptr_eq(j, pi[0]);
}

TEST_CASE_ABORT(ring_drop__invalid_magic)
{
    ring_drop((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_drop__empty, ring_new_empty, ring_free)
{
    test_int_error(ring_drop(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_drop, ring_new, ring_free)
{
    test_int_success(ring_drop(ring));
    test_uint_eq(ring_size(ring), 3);
    test_int_eq(*(int*)ring_peek(ring), i[1]);
}

TEST_CASE_ABORT(ring_drop_f__invalid_magic)
{
    ring_drop_f((ring_ct)&not_a_ring, test_ring_dtor, &count);
}

TEST_CASE_FIX(ring_drop_f__empty, ring_new_empty, ring_free)
{
    test_int_error(ring_drop_f(ring, test_ring_dtor, &count), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_drop_f, ring_new, ring_free)
{
    count = 0;
    test_int_success(ring_drop_f(ring, test_ring_dtor, &count));
    test_uint_eq(ring_size(ring), 3);
    test_int_eq(*(int*)ring_peek(ring), i[1]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(ring_get__invalid_magic)
{
    ring_get((ring_ct)&not_a_ring, &k);
}

TEST_CASE_FIX(ring_get__empty, ring_new_empty, ring_free)
{
    test_int_error(ring_get(ring, &k), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_get, ring_new, ring_free)
{
    test_int_success(ring_get(ring, &k));
    test_uint_eq(ring_size(ring), 3);
    test_int_eq(k, i[0]);
}

TEST_CASE_ABORT(ring_get_p__invalid_magic)
{
    ring_get_p((ring_ct)&not_a_ring);
}

TEST_CASE_FIX_ABORT(ring_get_p__invalid_type, ring_new, ring_free)
{
    ring_get_p(ring);
}

TEST_CASE_FIX(ring_get_p__empty, ring_new_ptr_empty, ring_free)
{
    test_ptr_error(ring_get_p(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_get_p, ring_new_ptr, ring_free)
{
    test_ptr_success(j = ring_get_p(ring));
    test_uint_eq(ring_size(ring), 3);
    test_ptr_eq(j, pi[0]);
}

TEST_CASE_ABORT(ring_peek_head__invalid_magic)
{
    ring_peek_head((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_peek_head__empty, ring_new_empty, ring_free)
{
    test_ptr_error(ring_peek_head(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_peek_head, ring_new, ring_free)
{
    test_ptr_success(j = ring_peek_head(ring));
    test_uint_eq(ring_size(ring), 4);
    test_int_eq(*j, i[3]);
}

TEST_CASE_ABORT(ring_peek_head_p__invalid_magic)
{
    ring_peek_head_p((ring_ct)&not_a_ring);
}

TEST_CASE_FIX_ABORT(ring_peek_head_p__invalid_type, ring_new, ring_free)
{
    ring_peek_head_p(ring);
}

TEST_CASE_FIX(ring_peek_head_p__empty, ring_new_ptr_empty, ring_free)
{
    test_ptr_error(ring_peek_head_p(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_peek_head_p, ring_new_ptr, ring_free)
{
    test_ptr_success(j = ring_peek_head_p(ring));
    test_uint_eq(ring_size(ring), 4);
    test_ptr_eq(j, pi[3]);
}

TEST_CASE_ABORT(ring_drop_head__invalid_magic)
{
    ring_drop_head((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_drop_head__empty, ring_new_empty, ring_free)
{
    test_int_error(ring_drop_head(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_drop_head, ring_new, ring_free)
{
    test_int_success(ring_drop_head(ring));
    test_uint_eq(ring_size(ring), 3);
    test_int_eq(*(int*)ring_peek_head(ring), i[2]);
}

TEST_CASE_ABORT(ring_drop_head_f__invalid_magic)
{
    ring_drop_head_f((ring_ct)&not_a_ring, test_ring_dtor, &count);
}

TEST_CASE_FIX(ring_drop_head_f__empty, ring_new_empty, ring_free)
{
    test_int_error(ring_drop_head_f(ring, test_ring_dtor, &count), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_drop_head_f, ring_new, ring_free)
{
    count = 0;
    test_int_success(ring_drop_head_f(ring, test_ring_dtor, &count));
    test_uint_eq(ring_size(ring), 3);
    test_int_eq(*(int*)ring_peek_head(ring), i[2]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(ring_get_head__invalid_magic)
{
    ring_get_head((ring_ct)&not_a_ring, &k);
}

TEST_CASE_FIX(ring_get_head__empty, ring_new_empty, ring_free)
{
    test_int_error(ring_get_head(ring, &k), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_get_head, ring_new, ring_free)
{
    test_int_success(ring_get_head(ring, &k));
    test_uint_eq(ring_size(ring), 3);
    test_int_eq(k, i[3]);
}

TEST_CASE_ABORT(ring_get_head_p__invalid_magic)
{
    ring_get_head_p((ring_ct)&not_a_ring);
}

TEST_CASE_FIX_ABORT(ring_get_head_p__invalid_type, ring_new, ring_free)
{
    ring_get_head_p(ring);
}

TEST_CASE_FIX(ring_get_head_p__empty, ring_new_ptr_empty, ring_free)
{
    test_ptr_error(ring_get_head_p(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_get_head_p, ring_new_ptr, ring_free)
{
    test_ptr_success(j = ring_get_head_p(ring));
    test_uint_eq(ring_size(ring), 3);
    test_ptr_eq(j, pi[3]);
}

static int test_ring_fold(ring_const_ct r, void *elem, void *ctx)
{
    int *i = elem, *sum = ctx;

    *sum = *sum*10 + *i;

    return 0;
}

TEST_CASE_ABORT(ring_fold__invalid_magic)
{
    ring_fold((ring_ct)&not_a_ring, test_ring_fold, NULL);
}

TEST_CASE_FIX_ABORT(ring_fold__invalid_callback, ring_new, ring_free)
{
    ring_fold(ring, NULL, NULL);
}

TEST_CASE_FIX(ring_fold, ring_new, ring_free)
{
    int sum = 0;
    test_int_success(ring_fold(ring, test_ring_fold, &sum));
    test_int_eq(sum, 1234);
}

TEST_CASE_ABORT(ring_fold_r__invalid_magic)
{
    ring_fold_r((ring_ct)&not_a_ring, test_ring_fold, NULL);
}

TEST_CASE_FIX_ABORT(ring_fold_r__invalid_callback, ring_new, ring_free)
{
    ring_fold_r(ring, NULL, NULL);
}

TEST_CASE_FIX(ring_fold_r, ring_new, ring_free)
{
    int sum = 0;
    test_int_success(ring_fold_r(ring, test_ring_fold, &sum));
    test_int_eq(sum, 4321);
}

int test_suite_con_ring(void *param)
{
    return error_pass_int(test_run_cases("ring",
        test_case(ring_new__invalid_elemsize),
        test_case(ring_new_c__invalid_elemsize),
        test_case(ring_free__invalid_magic),
        test_case(ring_free_f__invalid_magic),
        test_case(ring_free_f),
        test_case(ring_free_if_empty__invalid_magic),
        test_case(ring_free_if_empty__empty),
        test_case(ring_free_if_empty),
        test_case(ring_free_if_empty_f__invalid_magic),
        test_case(ring_free_if_empty_f__empty),
        test_case(ring_free_if_empty_f),
        test_case(ring_clear__invalid_magic),
        test_case(ring_clear),
        test_case(ring_clear_f__invalid_magic),
        test_case(ring_clear_f),
        test_case(ring_clone__invalid_magic),
        test_case(ring_clone__empty),
        test_case(ring_clone),
        test_case(ring_clone_f__invalid_magic),
        test_case(ring_clone_f__empty),
        test_case(ring_clone_f__shallow),
        test_case(ring_clone_f__deep),
        test_case(ring_clone_f__deep_fail),

        test_case(ring_is_empty__invalid_magic),
        test_case(ring_is_empty__null),
        test_case(ring_is_empty),
        test_case(ring_size__invalid_magic),
        test_case(ring_size__null),
        test_case(ring_size),
        test_case(ring_elemsize__invalid_magic),
        test_case(ring_elemsize),
        test_case(ring_capacity__invalid_magic),
        test_case(ring_capacity),
        test_case(ring_memsize__invalid_magic),
        test_case(ring_memsize_f__invalid_magic),

        test_case(ring_put__invalid_magic),
        test_case(ring_put),
        test_case(ring_put__overflow),
        test_case(ring_put_e__invalid_magic),
        test_case(ring_put_e),
        test_case(ring_put_e__overflow),
        test_case(ring_put_p__invalid_magic),
        test_case(ring_put_p__invalid_type),
        test_case(ring_put_p),
        test_case(ring_put_p__overflow),
        test_case(ring_put_overwrite__invalid_magic),
        test_case(ring_put_overwrite),
        test_case(ring_put_overwrite__overflow),
        test_case(ring_put_overwrite_e__invalid_magic),
        test_case(ring_put_overwrite_e),
        test_case(ring_put_overwrite_e__overflow),
        test_case(ring_put_overwrite_p__invalid_magic),
        test_case(ring_put_overwrite_p__invalid_type),
        test_case(ring_put_overwrite_p),
        test_case(ring_put_overwrite_p__overflow),

        test_case(ring_peek__invalid_magic),
        test_case(ring_peek__empty),
        test_case(ring_peek),
        test_case(ring_peek_p__invalid_magic),
        test_case(ring_peek_p__invalid_type),
        test_case(ring_peek_p__empty),
        test_case(ring_peek_p),
        test_case(ring_drop__invalid_magic),
        test_case(ring_drop__empty),
        test_case(ring_drop),
        test_case(ring_drop_f__invalid_magic),
        test_case(ring_drop_f__empty),
        test_case(ring_drop_f),
        test_case(ring_get__invalid_magic),
        test_case(ring_get__empty),
        test_case(ring_get),
        test_case(ring_get_p__invalid_magic),
        test_case(ring_get_p__invalid_type),
        test_case(ring_get_p__empty),
        test_case(ring_get_p),

        test_case(ring_peek_head__invalid_magic),
        test_case(ring_peek_head__empty),
        test_case(ring_peek_head),
        test_case(ring_peek_head_p__invalid_magic),
        test_case(ring_peek_head_p__invalid_type),
        test_case(ring_peek_head_p__empty),
        test_case(ring_peek_head_p),
        test_case(ring_drop_head__invalid_magic),
        test_case(ring_drop_head__empty),
        test_case(ring_drop_head),
        test_case(ring_drop_head_f__invalid_magic),
        test_case(ring_drop_head_f__empty),
        test_case(ring_drop_head_f),
        test_case(ring_get_head__invalid_magic),
        test_case(ring_get_head__empty),
        test_case(ring_get_head),
        test_case(ring_get_head_p__invalid_magic),
        test_case(ring_get_head_p__invalid_type),
        test_case(ring_get_head_p__empty),
        test_case(ring_get_head_p),

        test_case(ring_fold__invalid_magic),
        test_case(ring_fold__invalid_callback),
        test_case(ring_fold),
        test_case(ring_fold_r__invalid_magic),
        test_case(ring_fold_r__invalid_callback),
        test_case(ring_fold_r),

        NULL
    ));
}
