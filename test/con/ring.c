/*
 * Copyright (c) 2019 Martin Rödel a.k.a. Yomin Nimoy
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
static int count, *j, k;
static ring_action_id action;
static ring_ct ring;


TEST_SETUP(ring_new_empty)
{
    test_ptr_success(ring = ring_new(4, sizeof(int)));
}

TEST_SETUP(ring_new)
{
    test_ptr_success(ring = ring_new(4, sizeof(int)));
    test_ptr_success(ring_put_e(ring, &i[0]));
    test_ptr_success(ring_put_e(ring, &i[1]));
    test_ptr_success(ring_put_e(ring, &i[2]));
    test_ptr_success(ring_put_e(ring, &i[3]));
}

TEST_TEARDOWN(ring_free)
{
    test_void(ring_free(ring));
}

TEST_CASE(ring_new_invalid_elemsize)
{
    test_ptr_error(ring_new(0, 0), E_RING_INVALID_ELEMSIZE);
}

TEST_CASE_ABORT(ring_free_invalid_magic)
{
    ring_free((ring_ct)&not_a_ring);
}

static void test_ring_dtor(ring_const_ct r, void *elem, void *ctx)
{
    int *count = ctx;

    count[0]++;
}

TEST_CASE_ABORT(ring_free_f_invalid_magic)
{
    ring_free_f((ring_ct)&not_a_ring, test_ring_dtor, &count);
}

TEST_CASE_FIX(ring_free_f, ring_new, no_teardown)
{
    count = 0;
    test_void(ring_free_f(ring, test_ring_dtor, &count));
    test_int_eq(count, 4);
}

TEST_CASE_ABORT(ring_clear_invalid_magic)
{
    ring_clear((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_clear, ring_new, ring_free)
{
    test_void(ring_clear(ring));
    test_true(ring_is_empty(ring));
}

TEST_CASE_ABORT(ring_clear_f_invalid_magic)
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

TEST_CASE_ABORT(ring_is_empty_invalid_magic)
{
    ring_is_empty((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_is_empty, ring_new_empty, ring_free)
{
    test_true(ring_is_empty(ring));
    test_ptr_success(ring_put(ring));
    test_false(ring_is_empty(ring));
    test_int_success(ring_get(ring, NULL));
    test_true(ring_is_empty(ring));
}

TEST_CASE_ABORT(ring_size_invalid_magic)
{
    ring_size((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_size, ring_new_empty, ring_free)
{
    test_uint_eq(ring_size(ring), 0);
    test_ptr_success(ring_put(ring));
    test_uint_eq(ring_size(ring), 1);
    test_int_success(ring_get(ring, NULL));
    test_uint_eq(ring_size(ring), 0);
}

TEST_CASE_ABORT(ring_put_invalid_magic)
{
    ring_put((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_put, ring_new_empty, ring_free)
{
    test_ptr_success(ring_put(ring));
    test_uint_eq(ring_size(ring), 1);
}

TEST_CASE_FIX(ring_put_overflow, ring_new, ring_free)
{
    test_uint_eq(ring_size(ring), 4);
    test_ptr_error(ring_put(ring), E_RING_NO_SPACE);
    test_uint_eq(ring_size(ring), 4);
}

TEST_CASE_ABORT(ring_put_e_invalid_magic)
{
    ring_put_e((ring_ct)&not_a_ring, &i[0]);
}

TEST_CASE_FIX(ring_put_e, ring_new_empty, ring_free)
{
    test_ptr_success(j = ring_put_e(ring, &i[0]));
    test_uint_eq(ring_size(ring), 1);
    test_ptr_eq(j, ring_peek(ring));
    test_int_eq(*j, i[0]);
}

TEST_CASE_FIX(ring_put_e_overflow, ring_new, ring_free)
{
    test_uint_eq(ring_size(ring), 4);
    test_ptr_error(ring_put(ring), E_RING_NO_SPACE);
    test_uint_eq(ring_size(ring), 4);
}

static ring_action_id test_ring_overflow_reject(ring_const_ct r, const void *new_elem, void *old_elem, void *ctx)
{
    ring_action_id *action = ctx;

    *action = RING_REJECT;

    return RING_REJECT;
}

static ring_action_id test_ring_overflow_overwrite(ring_const_ct r, const void *new_elem, void *old_elem, void *ctx)
{
    ring_action_id *action = ctx;

    *action = RING_OVERWRITE;

    return RING_OVERWRITE;
}

TEST_CASE_ABORT(ring_put_f_invalid_magic)
{
    ring_put_f((ring_ct)&not_a_ring, test_ring_overflow_reject, &action);
}

TEST_CASE_FIX(ring_put_f, ring_new_empty, ring_free)
{
    action = 42;
    test_ptr_success(ring_put_f(ring, test_ring_overflow_reject, &action));
    test_uint_eq(action, 42);
    test_uint_eq(ring_size(ring), 1);
}

TEST_CASE_FIX(ring_put_f_overflow_reject, ring_new, ring_free)
{
    action = 42;
    test_ptr_error(ring_put_f(ring, test_ring_overflow_reject, &action), E_RING_NO_SPACE);
    test_uint_eq(action, RING_REJECT);
    test_uint_eq(ring_size(ring), 4);
}

TEST_CASE_FIX(ring_put_f_overflow_overwrite, ring_new, ring_free)
{
    action = 42;
    test_ptr_success(ring_put_f(ring, test_ring_overflow_overwrite, &action));
    test_uint_eq(action, RING_OVERWRITE);
    test_uint_eq(ring_size(ring), 4);
}

TEST_CASE_ABORT(ring_put_ef_invalid_magic)
{
    ring_put_ef((ring_ct)&not_a_ring, &i[0], test_ring_overflow_reject, &action);
}

TEST_CASE_FIX(ring_put_ef, ring_new_empty, ring_free)
{
    action = 42;
    test_ptr_success(j = ring_put_ef(ring, &i[0], test_ring_overflow_reject, &action));
    test_uint_eq(action, 42);
    test_uint_eq(ring_size(ring), 1);
    test_ptr_eq(j, ring_peek(ring));
    test_int_eq(*j, i[0]);
}

TEST_CASE_FIX(ring_put_ef_overflow_reject, ring_new, ring_free)
{
    action = 42;
    test_ptr_error(ring_put_ef(ring, &i[4], test_ring_overflow_reject, &action), E_RING_NO_SPACE);
    test_uint_eq(action, RING_REJECT);
    test_uint_eq(ring_size(ring), 4);
}

TEST_CASE_FIX(ring_put_ef_overflow_overwrite, ring_new, ring_free)
{
    action = 42;
    test_ptr_success(j = ring_put_ef(ring, &i[4], test_ring_overflow_overwrite, &action));
    test_uint_eq(action, RING_OVERWRITE);
    test_uint_eq(ring_size(ring), 4);
    test_int_eq(*(int*)ring_peek(ring), i[1]);
    test_int_eq(*(int*)ring_peek_head(ring), i[4]);
}

TEST_CASE_ABORT(ring_peek_invalid_magic)
{
    ring_peek((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_peek_empty, ring_new_empty, ring_free)
{
    test_ptr_error(ring_peek(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_peek, ring_new, ring_free)
{
    test_ptr_success(j = ring_peek(ring));
    test_int_eq(*j, i[0]);
}

TEST_CASE_ABORT(ring_drop_invalid_magic)
{
    ring_drop((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_drop_empty, ring_new_empty, ring_free)
{
    test_int_error(ring_drop(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_drop, ring_new, ring_free)
{
    test_int_success(ring_drop(ring));
    test_uint_eq(ring_size(ring), 3);
    test_int_eq(*(int*)ring_peek(ring), i[1]);
}

TEST_CASE_ABORT(ring_drop_f_invalid_magic)
{
    ring_drop_f((ring_ct)&not_a_ring, test_ring_dtor, &count);
}

TEST_CASE_FIX(ring_drop_f_empty, ring_new_empty, ring_free)
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

TEST_CASE_ABORT(ring_get_invalid_magic)
{
    ring_get((ring_ct)&not_a_ring, &k);
}

TEST_CASE_FIX(ring_get_empty, ring_new_empty, ring_free)
{
    test_int_error(ring_get(ring, &k), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_get, ring_new, ring_free)
{
    test_int_success(ring_get(ring, &k));
    test_uint_eq(ring_size(ring), 3);
    test_int_eq(k, i[0]);
}

TEST_CASE_ABORT(ring_peek_head_invalid_magic)
{
    ring_peek_head((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_peek_head_empty, ring_new_empty, ring_free)
{
    test_ptr_error(ring_peek_head(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_peek_head, ring_new, ring_free)
{
    test_ptr_success(j = ring_peek_head(ring));
    test_int_eq(*j, i[3]);
}

TEST_CASE_ABORT(ring_drop_head_invalid_magic)
{
    ring_drop_head((ring_ct)&not_a_ring);
}

TEST_CASE_FIX(ring_drop_head_empty, ring_new_empty, ring_free)
{
    test_int_error(ring_drop_head(ring), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_drop_head, ring_new, ring_free)
{
    test_int_success(ring_drop_head(ring));
    test_uint_eq(ring_size(ring), 3);
    test_int_eq(*(int*)ring_peek_head(ring), i[2]);
}

TEST_CASE_ABORT(ring_drop_head_f_invalid_magic)
{
    ring_drop_head_f((ring_ct)&not_a_ring, test_ring_dtor, &count);
}

TEST_CASE_FIX(ring_drop_head_f_empty, ring_new_empty, ring_free)
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

TEST_CASE_ABORT(ring_get_head_invalid_magic)
{
    ring_get_head((ring_ct)&not_a_ring, &k);
}

TEST_CASE_FIX(ring_get_head_empty, ring_new_empty, ring_free)
{
    test_int_error(ring_get_head(ring, &k), E_RING_EMPTY);
}

TEST_CASE_FIX(ring_get_head, ring_new, ring_free)
{
    test_int_success(ring_get_head(ring, &k));
    test_uint_eq(ring_size(ring), 3);
    test_int_eq(k, i[3]);
}

static int test_ring_fold(ring_const_ct r, void *elem, void *ctx)
{
    int *i = elem, *sum = ctx;

    *sum = *sum*10 + *i;

    return 0;
}

TEST_CASE_ABORT(ring_fold_invalid_magic)
{
    ring_fold((ring_ct)&not_a_ring, test_ring_fold, NULL);
}

TEST_CASE_FIX_ABORT(ring_fold_invalid_callback, ring_new, ring_free)
{
    ring_fold(ring, NULL, NULL);
}

TEST_CASE_FIX(ring_fold, ring_new, ring_free)
{
    int sum = 0;
    test_int_success(ring_fold(ring, test_ring_fold, &sum));
    test_int_eq(sum, 1234);
}

TEST_CASE_ABORT(ring_fold_r_invalid_magic)
{
    ring_fold_r((ring_ct)&not_a_ring, test_ring_fold, NULL);
}

TEST_CASE_FIX_ABORT(ring_fold_r_invalid_callback, ring_new, ring_free)
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
        test_case(ring_new_invalid_elemsize),
        test_case(ring_free_invalid_magic),
        test_case(ring_free_f_invalid_magic),
        test_case(ring_free_f),
        test_case(ring_clear_invalid_magic),
        test_case(ring_clear),
        test_case(ring_clear_f_invalid_magic),
        test_case(ring_clear_f),
        test_case(ring_is_empty_invalid_magic),
        test_case(ring_is_empty),
        test_case(ring_size_invalid_magic),
        test_case(ring_size),

        test_case(ring_put_invalid_magic),
        test_case(ring_put),
        test_case(ring_put_overflow),
        test_case(ring_put_e_invalid_magic),
        test_case(ring_put_e),
        test_case(ring_put_e_overflow),
        test_case(ring_put_f_invalid_magic),
        test_case(ring_put_f),
        test_case(ring_put_f_overflow_reject),
        test_case(ring_put_f_overflow_overwrite),
        test_case(ring_put_ef_invalid_magic),
        test_case(ring_put_ef),
        test_case(ring_put_ef_overflow_reject),
        test_case(ring_put_ef_overflow_overwrite),

        test_case(ring_peek_invalid_magic),
        test_case(ring_peek_empty),
        test_case(ring_peek),
        test_case(ring_drop_invalid_magic),
        test_case(ring_drop_empty),
        test_case(ring_drop),
        test_case(ring_drop_f_invalid_magic),
        test_case(ring_drop_f_empty),
        test_case(ring_drop_f),
        test_case(ring_get_invalid_magic),
        test_case(ring_get_empty),
        test_case(ring_get),

        test_case(ring_peek_head_invalid_magic),
        test_case(ring_peek_head_empty),
        test_case(ring_peek_head),
        test_case(ring_drop_head_invalid_magic),
        test_case(ring_drop_head_empty),
        test_case(ring_drop_head),
        test_case(ring_drop_head_f_invalid_magic),
        test_case(ring_drop_head_f_empty),
        test_case(ring_drop_head_f),
        test_case(ring_get_head_invalid_magic),
        test_case(ring_get_head_empty),
        test_case(ring_get_head),

        test_case(ring_fold_invalid_magic),
        test_case(ring_fold_invalid_callback),
        test_case(ring_fold),
        test_case(ring_fold_r_invalid_magic),
        test_case(ring_fold_r_invalid_callback),
        test_case(ring_fold_r),

        NULL
    ));
}
