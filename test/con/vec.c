/*
 * Copyright (c) 2019-2020 Martin Rödel a.k.a. Yomin Nimoy
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
#include <ytil/con/vec.h>
#include <string.h>
#include <stdlib.h>

static const struct not_a_vector
{
    int foo;
} not_a_vector = { 123 };

static const int i[] = { 1, 2, 33, 4, 5, 6, 7, 33, 9, 10 };
static const int *pi[] = { &i[0], &i[1], &i[2], &i[3], &i[4], &i[5], &i[6], &i[2], &i[8], &i[9] };
static int j[10], *k, **pk, count;
static vec_ct vec;


TEST_SETUP(vec_new_int)
{
    test_ptr_success(vec = vec_new(5, sizeof(int)));
    memset(j, 0, sizeof(j));
    count = 0;
}

TEST_SETUP(vec_new_int1)
{
    test_ptr_success(vec = vec_new(5, sizeof(int)));
    test_ptr_success(vec_push_e(vec, i));
    memset(j, 0, sizeof(j));
    count = 0;
}

TEST_SETUP(vec_new_int10)
{
    test_ptr_success(vec = vec_new(5, sizeof(int)));
    test_ptr_success(vec_push_en(vec, 10, i));
    memset(j, 0, sizeof(j));
    count = 0;
}

TEST_SETUP(vec_new_ptr)
{
    test_ptr_success(vec = vec_new(5, sizeof(int *)));
}

TEST_SETUP(vec_new_ptr1)
{
    test_ptr_success(vec = vec_new(5, sizeof(int *)));
    test_ptr_success(vec_push_p(vec, pi[0]));
}

TEST_SETUP(vec_new_ptr10)
{
    test_ptr_success(vec = vec_new(5, sizeof(int *)));
    test_ptr_success(vec_push_en(vec, 10, pi));
}

TEST_TEARDOWN(vec_free)
{
    test_void(vec_free(vec));
}

TEST_CASE_ABORT(vec_new_invalid_elemsize)
{
    test_void(vec_new(1, 0));
}

TEST_CASE_ABORT(vec_elemsize_invalid_magic)
{
    vec_elemsize((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIX(vec_elemsize, vec_new_int, vec_free)
{
    test_uint_eq(vec_elemsize(vec), sizeof(int));
}

TEST_CASE_ABORT(vec_is_empty_invalid_magic)
{
    vec_is_empty((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIX(vec_is_empty_on_init, vec_new_int, vec_free)
{
    test_true(vec_is_empty(vec));
}

TEST_CASE_FIX(vec_is_not_empty_after_push, vec_new_int, vec_free)
{
    test_ptr_success(vec_push(vec));
    test_false(vec_is_empty(vec));
}

TEST_CASE_FIX(vec_is_empty_after_push_pop, vec_new_int, vec_free)
{
    test_ptr_success(vec_push(vec));
    test_int_success(vec_pop(vec));
    test_true(vec_is_empty(vec));
}

TEST_CASE_ABORT(vec_size_invalid_magic)
{
    vec_size((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIX(vec_size_zero_on_init, vec_new_int, vec_free)
{
    test_uint_eq(vec_size(vec), 0);
}

TEST_CASE_FIX(vec_size_one_after_push, vec_new_int, vec_free)
{
    test_ptr_success(vec_push(vec));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_size_zero_after_push_pop, vec_new_int, vec_free)
{
    test_ptr_success(vec_push(vec));
    test_int_success(vec_pop(vec));
    test_uint_eq(vec_size(vec), 0);
}

TEST_CASE_ABORT(vec_capacity_invalid_magic)
{
    vec_capacity((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIX(vec_capacity_zero_on_init, vec_new_int, vec_free)
{
    test_uint_eq(vec_capacity(vec), 0);
}

TEST_CASE_FIX(vec_capacity_initial_capacity_after_push, vec_new_int, vec_free)
{
    test_ptr_success(vec_push(vec));
    test_uint_gt(vec_capacity(vec), 0);
}

TEST_CASE_FIX(vec_capacity_double_capacity_after_push_above_capacity, vec_new_int, vec_free)
{
    size_t cap;

    test_ptr_success(vec_push(vec));
    cap = vec_capacity(vec);
    test_ptr_success(vec_push_n(vec, cap));

    test_uint_eq(vec_capacity(vec), 2 * cap);
}

TEST_CASE_FIX(vec_capacity_initial_capacity_after_pop_below_threshold, vec_new_int, vec_free)
{
    size_t cap;

    test_ptr_success(vec_push(vec));
    cap = vec_capacity(vec);
    test_ptr_success(vec_push_n(vec, cap));
    test_int_success(vec_pop_n(vec, cap));

    test_uint_eq(vec_capacity(vec), cap);
}

TEST_CASE_ABORT(vec_pos_invalid_magic)
{
    vec_pos((vec_const_ct)&not_a_vector, NULL);
}

TEST_CASE_FIX_ABORT(vec_pos_invalid_elem, vec_new_int, vec_free)
{
    vec_pos(vec, NULL);
}

TEST_CASE_FIX(vec_pos_no_member, vec_new_int, vec_free)
{
    test_int_error(vec_pos(vec, &not_a_vector), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_pos, vec_new_int10, vec_free)
{
    int *i = vec_first(vec);

    test_rc_success(vec_pos(vec, i + 5), 5, -1);
}

TEST_CASE_FIX(vec_pos_no_member_before, vec_new_int10, vec_free)
{
    int *i = vec_first(vec);

    test_int_error(vec_pos(vec, i - 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_pos_no_member_after, vec_new_int10, vec_free)
{
    int *i = vec_last(vec);

    test_int_error(vec_pos(vec, i + 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_pos_member_invalid_alignment, vec_new_int10, vec_free)
{
    int *i = vec_first(vec);

    vec_pos(vec, ((char *)i) + 1);
}

TEST_CASE_ABORT(vec_first_invalid_magic)
{
    vec_first((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIX(vec_first_empty_vector, vec_new_int, vec_free)
{
    test_ptr_error(vec_first(vec), E_VEC_EMPTY);
}

TEST_CASE_FIX(vec_first, vec_new_int10, vec_free)
{
    test_int_eq(*(int *)vec_first(vec), i[0]);
}

TEST_CASE_ABORT(vec_first_p_invalid_magic)
{
    vec_first_p((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIX_ABORT(vec_first_p_invalid_type, vec_new_int, vec_free)
{
    vec_first_p(vec);
}

TEST_CASE_FIX(vec_first_p_empty_vector, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_first_p(vec), E_VEC_EMPTY);
}

TEST_CASE_FIX(vec_first_p, vec_new_ptr10, vec_free)
{
    test_ptr_eq(vec_first_p(vec), pi[0]);
}

TEST_CASE_ABORT(vec_last_invalid_magic)
{
    vec_last((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIX(vec_last_empty_vector, vec_new_int, vec_free)
{
    test_ptr_error(vec_last(vec), E_VEC_EMPTY);
}

TEST_CASE_FIX(vec_last, vec_new_int10, vec_free)
{
    test_int_eq(*(int *)vec_last(vec), i[9]);
}

TEST_CASE_ABORT(vec_last_p_invalid_magic)
{
    vec_last_p((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIX_ABORT(vec_last_p_invalid_type, vec_new_int, vec_free)
{
    vec_last_p(vec);
}

TEST_CASE_FIX(vec_last_p_empty_vector, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_last_p(vec), E_VEC_EMPTY);
}

TEST_CASE_FIX(vec_last_p, vec_new_ptr10, vec_free)
{
    test_ptr_eq(vec_last_p(vec), pi[9]);
}

TEST_CASE_ABORT(vec_at_invalid_magic)
{
    vec_at((vec_const_ct)&not_a_vector, 0);
}

TEST_CASE_FIX(vec_at_empty_vector_positive_index, vec_new_int, vec_free)
{
    test_ptr_error(vec_at(vec, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_at_empty_vector_negative_index, vec_new_int, vec_free)
{
    test_ptr_error(vec_at(vec, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_at_positive_index, vec_new_int10, vec_free)
{
    test_int_eq(*(int *)vec_at(vec, 4), i[4]);
}

TEST_CASE_FIX(vec_at_negative_index, vec_new_int10, vec_free)
{
    test_int_eq(*(int *)vec_at(vec, -4), i[6]);
}

TEST_CASE_ABORT(vec_at_p_invalid_magic)
{
    vec_at_p((vec_const_ct)&not_a_vector, 0);
}

TEST_CASE_FIX_ABORT(vec_at_p_invalid_type, vec_new_int, vec_free)
{
    vec_at_p(vec, 0);
}

TEST_CASE_FIX(vec_at_p_empty_vector_positive_index, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_at_p(vec, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_at_p_empty_vector_negative_index, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_at_p(vec, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_at_p_positive_index, vec_new_ptr10, vec_free)
{
    test_ptr_eq(vec_at_p(vec, 4), pi[4]);
}

TEST_CASE_FIX(vec_at_p_negative_index, vec_new_ptr10, vec_free)
{
    test_ptr_eq(vec_at_p(vec, -4), pi[6]);
}

TEST_CASE_ABORT(vec_get_first_invalid_magic)
{
    vec_get_first((vec_const_ct)&not_a_vector, NULL);
}

TEST_CASE_FIX(vec_get_first_empty_vector, vec_new_int, vec_free)
{
    test_int_error(vec_get_first(vec, j), E_VEC_EMPTY);
}

TEST_CASE_FIX(vec_get_first, vec_new_int10, vec_free)
{
    test_int_success(vec_get_first(vec, j));
    test_int_eq(j[0], i[0]);
}

TEST_CASE_ABORT(vec_get_last_invalid_magic)
{
    vec_get_last((vec_const_ct)&not_a_vector, NULL);
}

TEST_CASE_FIX(vec_get_last_empty_vector, vec_new_int, vec_free)
{
    test_int_error(vec_get_last(vec, j), E_VEC_EMPTY);
}

TEST_CASE_FIX(vec_get_last, vec_new_int10, vec_free)
{
    test_int_success(vec_get_last(vec, j));
    test_int_eq(j[0], i[9]);
}

TEST_CASE_ABORT(vec_get_invalid_magic)
{
    vec_get((vec_const_ct)&not_a_vector, NULL, 0);
}

TEST_CASE_FIX(vec_get_empty_vector_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_get(vec, j, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_get_empty_vector_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_get(vec, j, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_get_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_get(vec, j, 4));
    test_int_eq(j[0], i[4]);
}

TEST_CASE_FIX(vec_get_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_get(vec, j, -4));
    test_int_eq(j[0], i[6]);
}

TEST_CASE_ABORT(vec_get_n_invalid_magic)
{
    vec_get_n((vec_const_ct)&not_a_vector, NULL, 0, 1);
}

TEST_CASE_FIX(vec_get_n_empty_vector_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_get_n(vec, j, 1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_get_n_empty_vector_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_get_n(vec, j, -1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_get_n_nothing_positive_index, vec_new_int1, vec_free)
{
    j[0] = 123;
    test_int_success(vec_get_n(vec, j, 0, 0));
    test_int_eq(j[0], 123);
}

TEST_CASE_FIX(vec_get_n_nothing_negative_index, vec_new_int1, vec_free)
{
    j[0] = 123;
    test_int_success(vec_get_n(vec, j, -1, 0));
    test_int_eq(j[0], 123);
}

TEST_CASE_FIX(vec_get_n_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_get_n(vec, j, 4, 2));
    test_int_list_eq(j, i + 4, 2);
}

TEST_CASE_FIX(vec_get_n_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_get_n(vec, j, -4, 2));
    test_int_list_eq(j, i + 6, 2);
}

TEST_CASE_ABORT(vec_push_invalid_magic)
{
    vec_push((vec_ct)&not_a_vector);
}

TEST_CASE_FIX(vec_push, vec_new_int, vec_free)
{
    test_ptr_success(vec_push(vec));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_ABORT(vec_push_e_invalid_magic)
{
    vec_push_e((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_FIX(vec_push_e, vec_new_int, vec_free)
{
    test_ptr_success(vec_push_e(vec, i));
    test_uint_eq(vec_size(vec), 1);
    test_int_eq(*(int *)vec_at(vec, 0), i[0]);
}

TEST_CASE_ABORT(vec_push_p_invalid_magic)
{
    vec_push_p((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_FIX_ABORT(vec_push_p_invalid_type, vec_new_int, vec_free)
{
    vec_push_p(vec, NULL);
}

TEST_CASE_FIX(vec_push_p, vec_new_ptr, vec_free)
{
    test_ptr_success(vec_push_p(vec, pi[0]));
    test_uint_eq(vec_size(vec), 1);
    test_ptr_eq(vec_at_p(vec, 0), pi[0]);
}

TEST_CASE_ABORT(vec_push_n_invalid_magic)
{
    vec_push_n((vec_ct)&not_a_vector, 2);
}

TEST_CASE_FIX(vec_push_n_nothing, vec_new_int1, vec_free)
{
    test_ptr_success(vec_push_n(vec, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_push_n, vec_new_int, vec_free)
{
    test_ptr_success(vec_push_n(vec, 2));
    test_uint_eq(vec_size(vec), 2);
}

TEST_CASE_ABORT(vec_push_en_invalid_magic)
{
    vec_push_en((vec_ct)&not_a_vector, 2, NULL);
}

TEST_CASE_FIX(vec_push_en_nothing, vec_new_int1, vec_free)
{
    test_ptr_success(vec_push_en(vec, 0, NULL));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_push_en, vec_new_int, vec_free)
{
    test_ptr_success(vec_push_en(vec, 2, i));
    test_uint_eq(vec_size(vec), 2);
    test_int_list_eq((int *)vec_first(vec), i, 2);
}

TEST_CASE_ABORT(vec_push_a_invalid_magic)
{
    vec_push_a((vec_ct)&not_a_vector, 0);
}

TEST_CASE_FIX(vec_push_a_nothing, vec_new_int1, vec_free)
{
    test_ptr_success(vec_push_a(vec, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_push_a, vec_new_int, vec_free)
{
    test_ptr_success(vec_push_a(vec, 2, &i[0], &i[1]));
    test_uint_eq(vec_size(vec), 2);
    test_int_list_eq((int *)vec_first(vec), i, 2);
}

TEST_CASE_ABORT(vec_push_ap_invalid_magic)
{
    vec_push_ap((vec_ct)&not_a_vector, 0);
}

TEST_CASE_FIX_ABORT(vec_push_ap_invalid_type, vec_new_int, vec_free)
{
    vec_push_ap(vec, 0);
}

TEST_CASE_FIX(vec_push_ap_nothing, vec_new_ptr1, vec_free)
{
    test_ptr_success(vec_push_ap(vec, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_push_ap, vec_new_ptr, vec_free)
{
    test_ptr_success(vec_push_ap(vec, 2, pi[0], pi[1]));
    test_uint_eq(vec_size(vec), 2);
    test_ptr_list_eq((int **)vec_first(vec), pi, 2);
}

TEST_CASE_ABORT(vec_insert_invalid_magic)
{
    vec_insert((vec_ct)&not_a_vector, 0);
}

TEST_CASE_FIX(vec_insert_oob_positive_index, vec_new_int, vec_free)
{
    test_ptr_error(vec_insert(vec, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_insert_oob_negative_index, vec_new_int, vec_free)
{
    test_ptr_error(vec_insert(vec, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_insert_front_positive_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert(vec, 0));
    test_int_eq(vec_pos(vec, k), 0);
    test_int_eq(*(int *)vec_at(vec, 1), i[0]);
}

TEST_CASE_FIX(vec_insert_back_positive_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert(vec, 10));
    test_int_eq(vec_pos(vec, k), 10);
}

TEST_CASE_FIX(vec_insert_front_negative_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert(vec, -10));
    test_int_eq(vec_pos(vec, k), 0);
    test_int_eq(*(int *)vec_at(vec, 1), i[0]);
}

TEST_CASE_FIX(vec_insert_back_negative_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert(vec, -1));
    test_int_eq(vec_pos(vec, k), 9); // -1 inserts before last elem
    test_int_eq(*(int *)vec_at(vec, 10), i[9]);
}

TEST_CASE_ABORT(vec_insert_e_invalid_magic)
{
    vec_insert_e((vec_ct)&not_a_vector, 0, NULL);
}

TEST_CASE_FIX(vec_insert_e_oob_positive_index, vec_new_int, vec_free)
{
    test_ptr_error(vec_insert_e(vec, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_insert_e_oob_negative_index, vec_new_int, vec_free)
{
    test_ptr_error(vec_insert_e(vec, -1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_insert_e_front_positive_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_e(vec, 0, &i[5]));
    test_int_eq(vec_pos(vec, k), 0);
    test_int_eq(*k, i[5]);
    test_int_eq(*(int *)vec_at(vec, 1), i[0]);
}

TEST_CASE_FIX(vec_insert_e_back_positive_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_e(vec, 10, &i[5]));
    test_int_eq(vec_pos(vec, k), 10);
    test_int_eq(*k, i[5]);
}

TEST_CASE_FIX(vec_insert_e_front_negative_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_e(vec, -10, &i[5]));
    test_int_eq(vec_pos(vec, k), 0);
    test_int_eq(*k, i[5]);
    test_int_eq(*(int *)vec_at(vec, 1), i[0]);
}

TEST_CASE_FIX(vec_insert_e_back_negative_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_e(vec, -1, &i[5]));
    test_int_eq(vec_pos(vec, k), 9); // -1 inserts before last elem
    test_int_eq(*k, i[5]);
    test_int_eq(*(int *)vec_at(vec, 10), i[9]);
}

TEST_CASE_ABORT(vec_insert_p_invalid_magic)
{
    vec_insert_p((vec_ct)&not_a_vector, 0, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_p_invalid_type, vec_new_int, vec_free)
{
    vec_insert_p(vec, 0, NULL);
}

TEST_CASE_FIX(vec_insert_p_oob_positive_index, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_insert_p(vec, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_insert_p_oob_negative_index, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_insert_p(vec, -1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_insert_p_front_positive_index, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_insert_p(vec, 0, pi[5]));
    test_int_eq(vec_pos(vec, pk), 0);
    test_ptr_eq(*pk, pi[5]);
    test_ptr_eq(vec_at_p(vec, 1), pi[0]);
}

TEST_CASE_FIX(vec_insert_p_back_positive_index, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_insert_p(vec, 10, pi[5]));
    test_int_eq(vec_pos(vec, pk), 10);
    test_ptr_eq(*pk, pi[5]);
}

TEST_CASE_FIX(vec_insert_p_front_negative_index, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_insert_p(vec, -10, pi[5]));
    test_int_eq(vec_pos(vec, pk), 0);
    test_ptr_eq(*pk, pi[5]);
    test_ptr_eq(vec_at_p(vec, 1), pi[0]);
}

TEST_CASE_FIX(vec_insert_p_back_negative_index, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_insert_p(vec, -1, pi[5]));
    test_int_eq(vec_pos(vec, pk), 9); // -1 inserts before last elem
    test_ptr_eq(*pk, pi[5]);
    test_ptr_eq(vec_at_p(vec, 10), pi[9]);
}

TEST_CASE_ABORT(vec_insert_n_invalid_magic)
{
    vec_insert_n((vec_ct)&not_a_vector, 0, 1);
}

TEST_CASE_FIX(vec_insert_n_oob_positive_index, vec_new_int, vec_free)
{
    test_ptr_error(vec_insert_n(vec, 1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_insert_n_nothing_positive_index, vec_new_int1, vec_free)
{
    test_ptr_success(vec_insert_n(vec, 0, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_insert_n_nothing_negative_index, vec_new_int1, vec_free)
{
    test_ptr_success(vec_insert_n(vec, -1, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_insert_n_oob_negative_index, vec_new_int, vec_free)
{
    test_ptr_error(vec_insert_n(vec, -1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_insert_n_front_positive_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_n(vec, 0, 2));
    test_int_eq(vec_pos(vec, k), 0);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int *)vec_at(vec, 2), i[0]);
}

TEST_CASE_FIX(vec_insert_n_back_positive_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_n(vec, 10, 2));
    test_int_eq(vec_pos(vec, k), 10);
    test_uint_eq(vec_size(vec), 12);
}

TEST_CASE_FIX(vec_insert_n_front_negative_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_n(vec, -10, 2));
    test_int_eq(vec_pos(vec, k), 0);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int *)vec_at(vec, 2), i[0]);
}

TEST_CASE_FIX(vec_insert_n_back_negative_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_n(vec, -1, 2));
    test_int_eq(vec_pos(vec, k), 9); // -1 inserts before last elem
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int *)vec_at(vec, 11), i[9]);
}

TEST_CASE_ABORT(vec_insert_en_invalid_magic)
{
    vec_insert_en((vec_ct)&not_a_vector, 0, 1, NULL);
}

TEST_CASE_FIX(vec_insert_en_oob_positive_index, vec_new_int, vec_free)
{
    test_ptr_error(vec_insert_en(vec, 1, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_insert_en_oob_negative_index, vec_new_int, vec_free)
{
    test_ptr_error(vec_insert_en(vec, -1, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_insert_en_nothing_positive_index, vec_new_int1, vec_free)
{
    test_ptr_success(vec_insert_en(vec, 0, 0, NULL));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_insert_en_nothing_negative_index, vec_new_int1, vec_free)
{
    test_ptr_success(vec_insert_en(vec, -1, 0, NULL));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_insert_en_front_positive_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_en(vec, 0, 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 0);
    test_uint_eq(vec_size(vec), 12);
    test_int_list_eq(k, &i[5], 2);
    test_int_eq(*(int *)vec_at(vec, 2), i[0]);
}

TEST_CASE_FIX(vec_insert_en_back_positive_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_en(vec, 10, 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 10);
    test_uint_eq(vec_size(vec), 12);
    test_int_list_eq(k, &i[5], 2);
}

TEST_CASE_FIX(vec_insert_en_front_negative_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_en(vec, -10, 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 0);
    test_uint_eq(vec_size(vec), 12);
    test_int_list_eq(k, &i[5], 2);
    test_int_eq(*(int *)vec_at(vec, 2), i[0]);
}

TEST_CASE_FIX(vec_insert_en_back_negative_index, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_en(vec, -1, 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 9); // -1 inserts before last elem
    test_uint_eq(vec_size(vec), 12);
    test_int_list_eq(k, &i[5], 2);
    test_int_eq(*(int *)vec_at(vec, 11), i[9]);
}

TEST_CASE_ABORT(vec_insert_before_invalid_magic)
{
    vec_insert_before((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_before_invalid_elem, vec_new_int, vec_free)
{
    vec_insert_before(vec, NULL);
}

TEST_CASE_FIX(vec_insert_before_no_member, vec_new_int10, vec_free)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_before(vec, k - 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_insert_before_member_invalid_alignment, vec_new_int10, vec_free)
{
    k = vec_at(vec, 0);
    vec_insert_before(vec, ((char *)k) + 1);
}

TEST_CASE_FIX(vec_insert_before, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_before(vec, vec_at(vec, 1)));
    test_int_eq(vec_pos(vec, k), 1);
    test_uint_eq(vec_size(vec), 11);
    test_int_eq(*(int *)vec_at(vec, 2), i[1]);
}

TEST_CASE_ABORT(vec_insert_before_e_invalid_magic)
{
    vec_insert_before_e((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_before_e_invalid_elem, vec_new_int, vec_free)
{
    vec_insert_before_e(vec, NULL, NULL);
}

TEST_CASE_FIX(vec_insert_before_e_no_member, vec_new_int10, vec_free)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_before_e(vec, k - 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_insert_before_e_member_invalid_alignment, vec_new_int10, vec_free)
{
    k = vec_at(vec, 0);
    vec_insert_before_e(vec, ((char *)k) + 1, NULL);
}

TEST_CASE_FIX(vec_insert_before_e, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_before_e(vec, vec_at(vec, 1), &i[5]));
    test_int_eq(vec_pos(vec, k), 1);
    test_int_eq(*k, i[5]);
    test_uint_eq(vec_size(vec), 11);
    test_int_eq(*(int *)vec_at(vec, 2), i[1]);
}

TEST_CASE_ABORT(vec_insert_before_p_invalid_magic)
{
    vec_insert_before_p((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_before_p_invalid_elem, vec_new_ptr, vec_free)
{
    vec_insert_before_p(vec, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_before_p_invalid_type, vec_new_int, vec_free)
{
    vec_insert_before_p(vec, vec_at(vec, 0), NULL);
}

TEST_CASE_FIX(vec_insert_before_p_no_member, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_at(vec, 0));
    test_ptr_error(vec_insert_before_p(vec, pk - 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_insert_before_p_member_invalid_alignment, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_at(vec, 0));
    vec_insert_before_p(vec, ((char *)pk) + 1, NULL);
}

TEST_CASE_FIX(vec_insert_before_p, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_insert_before_p(vec, vec_at(vec, 1), pi[5]));
    test_int_eq(vec_pos(vec, pk), 1);
    test_ptr_eq(*pk, pi[5]);
    test_uint_eq(vec_size(vec), 11);
    test_ptr_eq(vec_at_p(vec, 2), pi[1]);
}

TEST_CASE_ABORT(vec_insert_before_n_invalid_magic)
{
    vec_insert_before_n((vec_ct)&not_a_vector, NULL, 1);
}

TEST_CASE_FIX_ABORT(vec_insert_before_n_invalid_elem, vec_new_int, vec_free)
{
    vec_insert_before_n(vec, NULL, 1);
}

TEST_CASE_FIX(vec_insert_before_n_no_member, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    test_ptr_error(vec_insert_before_n(vec, k - 1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_insert_before_n_member_invalid_alignment, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    vec_insert_before_n(vec, ((char *)k) + 1, 1);
}

TEST_CASE_FIX(vec_insert_before_n_nothing, vec_new_int1, vec_free)
{
    test_ptr_success(vec_insert_before_n(vec, vec_first(vec), 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_insert_before_n, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_before_n(vec, vec_at(vec, 1), 2));
    test_int_eq(vec_pos(vec, k), 1);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int *)vec_at(vec, 3), i[1]);
}

TEST_CASE_ABORT(vec_insert_before_en_invalid_magic)
{
    vec_insert_before_en((vec_ct)&not_a_vector, NULL, 1, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_before_en_invalid_elem, vec_new_int, vec_free)
{
    vec_insert_before_en(vec, NULL, 1, NULL);
}

TEST_CASE_FIX(vec_insert_before_en_no_member, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    test_ptr_error(vec_insert_before_en(vec, k - 1, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_insert_before_en_member_invalid_alignment, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    vec_insert_before_en(vec, ((char *)k) + 1, 1, NULL);
}

TEST_CASE_FIX(vec_insert_before_en_nothing, vec_new_int1, vec_free)
{
    test_ptr_success(vec_insert_before_en(vec, vec_first(vec), 0, NULL));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_insert_before_en, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_before_en(vec, vec_at(vec, 1), 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 1);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int *)vec_at(vec, 3), i[1]);
    test_int_list_eq(k, &i[5], 2);
}

TEST_CASE_ABORT(vec_insert_after_invalid_magic)
{
    vec_insert_after((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_after_invalid_elem, vec_new_int, vec_free)
{
    vec_insert_after(vec, NULL);
}

TEST_CASE_FIX(vec_insert_after_no_member, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    test_ptr_error(vec_insert_after(vec, k - 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_insert_after_member_invalid_alignment, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    vec_insert_after(vec, ((char *)k) + 1);
}

TEST_CASE_FIX(vec_insert_after, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_after(vec, vec_at(vec, 1)));
    test_int_eq(vec_pos(vec, k), 2);
    test_uint_eq(vec_size(vec), 11);
    test_int_eq(*(int *)vec_at(vec, 3), i[2]);
}

TEST_CASE_ABORT(vec_insert_after_e_invalid_magic)
{
    vec_insert_after_e((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_after_e_invalid_elem, vec_new_int, vec_free)
{
    vec_insert_after_e(vec, NULL, NULL);
}

TEST_CASE_FIX(vec_insert_after_e_no_member, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    test_ptr_error(vec_insert_after_e(vec, k - 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_insert_after_e_member_invalid_alignment, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    vec_insert_after_e(vec, ((char *)k) + 1, NULL);
}

TEST_CASE_FIX(vec_insert_after_e, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_after_e(vec, vec_at(vec, 1), &i[5]));
    test_int_eq(vec_pos(vec, k), 2);
    test_int_eq(*k, i[5]);
    test_uint_eq(vec_size(vec), 11);
    test_int_eq(*(int *)vec_at(vec, 3), i[2]);
}

TEST_CASE_ABORT(vec_insert_after_p_invalid_magic)
{
    vec_insert_after_p((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_after_p_invalid_elem, vec_new_ptr, vec_free)
{
    vec_insert_after_p(vec, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_after_p_invalid_type, vec_new_int, vec_free)
{
    vec_insert_after_p(vec, vec_at(vec, 0), NULL);
}

TEST_CASE_FIX(vec_insert_after_p_no_member, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_at(vec, 0));
    test_ptr_error(vec_insert_after_p(vec, pk - 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_insert_after_p_member_invalid_alignment, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_at(vec, 0));
    vec_insert_after_p(vec, ((char *)pk) + 1, NULL);
}

TEST_CASE_FIX(vec_insert_after_p, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_insert_after_p(vec, vec_at(vec, 1), pi[5]));
    test_int_eq(vec_pos(vec, pk), 2);
    test_ptr_eq(*pk, pi[5]);
    test_uint_eq(vec_size(vec), 11);
    test_ptr_eq(vec_at_p(vec, 3), pi[2]);
}

TEST_CASE_ABORT(vec_insert_after_n_invalid_magic)
{
    vec_insert_after_n((vec_ct)&not_a_vector, NULL, 1);
}

TEST_CASE_FIX_ABORT(vec_insert_after_n_invalid_elem, vec_new_int, vec_free)
{
    vec_insert_after_n(vec, NULL, 1);
}

TEST_CASE_FIX(vec_insert_after_n_no_member, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    test_ptr_error(vec_insert_after_n(vec, k - 1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_insert_after_n_member_invalid_alignment, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    vec_insert_after_n(vec, ((char *)k) + 1, 1);
}

TEST_CASE_FIX(vec_insert_after_n_nothing, vec_new_int1, vec_free)
{
    test_ptr_success(vec_insert_after_n(vec, vec_first(vec), 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_insert_after_n, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_after_n(vec, vec_at(vec, 1), 2));
    test_int_eq(vec_pos(vec, k), 2);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int *)vec_at(vec, 4), i[2]);
}

TEST_CASE_ABORT(vec_insert_after_en_invalid_magic)
{
    vec_insert_after_en((vec_ct)&not_a_vector, NULL, 1, NULL);
}

TEST_CASE_FIX_ABORT(vec_insert_after_en_invalid_elem, vec_new_int, vec_free)
{
    vec_insert_after_en(vec, NULL, 1, NULL);
}

TEST_CASE_FIX(vec_insert_after_en_no_member, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    test_ptr_error(vec_insert_after_en(vec, k - 1, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_insert_after_en_member_invalid_alignment, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_at(vec, 0));
    vec_insert_after_en(vec, ((char *)k) + 1, 1, NULL);
}

TEST_CASE_FIX(vec_insert_after_en_nothing, vec_new_int1, vec_free)
{
    test_ptr_success(vec_insert_after_en(vec, vec_first(vec), 0, NULL));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_insert_after_en, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_insert_after_en(vec, vec_at(vec, 1), 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 2);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int *)vec_at(vec, 4), i[2]);
    test_int_list_eq(k, &i[5], 2);
}

TEST_CASE_ABORT(vec_set_invalid_magic)
{
    vec_set((vec_ct)&not_a_vector, 0, NULL);
}

TEST_CASE_FIX(vec_set_oob_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_set(vec, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_set_oob_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_set(vec, -1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_set_front_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_set(vec, 0, &i[2]));
    test_uint_eq(vec_size(vec), 10);
    test_int_eq(*(int *)vec_at(vec, 0), i[2]);
}

TEST_CASE_FIX(vec_set_back_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_set(vec, 9, &i[2]));
    test_uint_eq(vec_size(vec), 10);
    test_int_eq(*(int *)vec_at(vec, 9), i[2]);
}

TEST_CASE_FIX(vec_set_front_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_set(vec, -10, &i[2]));
    test_uint_eq(vec_size(vec), 10);
    test_int_eq(*(int *)vec_at(vec, 0), i[2]);
}

TEST_CASE_FIX(vec_set_back_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_set(vec, -1, &i[2]));
    test_uint_eq(vec_size(vec), 10);
    test_int_eq(*(int *)vec_at(vec, 9), i[2]);
}

static void test_vec_set_dtor(vec_const_ct v, void *elem, void *ctx)
{
    int *last = ctx, *i = elem;

    *last = *i;
}

TEST_CASE_ABORT(vec_set_f_invalid_magic)
{
    vec_set_f((vec_ct)&not_a_vector, 0, NULL, NULL, NULL);
}

TEST_CASE_FIX(vec_set_f_oob_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_set_f(vec, 1, NULL, NULL, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_set_f_oob_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_set_f(vec, -1, NULL, NULL, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_set_f_front_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_set_f(vec, 0, &i[2], test_vec_set_dtor, j));
    test_uint_eq(vec_size(vec), 10);
    test_int_eq(*(int *)vec_at(vec, 0), i[2]);
    test_int_eq(j[0], i[0]);
}

TEST_CASE_FIX(vec_set_f_back_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_set_f(vec, 9, &i[2], test_vec_set_dtor, j));
    test_uint_eq(vec_size(vec), 10);
    test_int_eq(*(int *)vec_at(vec, 9), i[2]);
    test_int_eq(j[0], i[9]);
}

TEST_CASE_FIX(vec_set_f_front_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_set_f(vec, -10, &i[2], test_vec_set_dtor, j));
    test_uint_eq(vec_size(vec), 10);
    test_int_eq(*(int *)vec_at(vec, 0), i[2]);
    test_int_eq(j[0], i[0]);
}

TEST_CASE_FIX(vec_set_f_back_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_set_f(vec, -1, &i[2], test_vec_set_dtor, j));
    test_uint_eq(vec_size(vec), 10);
    test_int_eq(*(int *)vec_at(vec, 9), i[2]);
    test_int_eq(j[0], i[9]);
}

TEST_CASE_ABORT(vec_pop_invalid_magic)
{
    vec_pop((vec_ct)&not_a_vector);
}

TEST_CASE_FIX(vec_pop_empty_vector, vec_new_int, vec_free)
{
    test_int_error(vec_pop(vec), E_VEC_EMPTY);
}

TEST_CASE_FIX(vec_pop, vec_new_int10, vec_free)
{
    test_int_success(vec_pop(vec));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_last(vec), i[8]);
}

TEST_CASE_ABORT(vec_pop_e_invalid_magic)
{
    vec_pop_e((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_FIX(vec_pop_e_empty_vector, vec_new_int, vec_free)
{
    test_int_error(vec_pop_e(vec, NULL), E_VEC_EMPTY);
}

TEST_CASE_FIX(vec_pop_e, vec_new_int10, vec_free)
{
    test_int_success(vec_pop_e(vec, j));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_last(vec), i[8]);
    test_int_eq(j[0], i[9]);
}

TEST_CASE_ABORT(vec_pop_p_invalid_magic)
{
    vec_pop_p((vec_ct)&not_a_vector);
}

TEST_CASE_FIX_ABORT(vec_pop_p_invalid_type, vec_new_int, vec_free)
{
    vec_pop_p(vec);
}

TEST_CASE_FIX(vec_pop_p_empty_vector, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_pop_p(vec), E_VEC_EMPTY);
}

TEST_CASE_FIX(vec_pop_p, vec_new_ptr10, vec_free)
{
    test_ptr_success(k = vec_pop_p(vec));
    test_uint_eq(vec_size(vec), 9);
    test_ptr_eq(*(int **)vec_last(vec), pi[8]);
    test_ptr_eq(k, pi[9]);
}

static void test_vec_dtor(vec_const_ct v, void *elem, void *ctx)
{
    int *count = ctx;

    count[0]++;
}

TEST_CASE_ABORT(vec_pop_f_invalid_magic)
{
    vec_pop_f((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX(vec_pop_f_empty_vector, vec_new_int, vec_free)
{
    test_int_error(vec_pop_f(vec, test_vec_dtor, &count), E_VEC_EMPTY);
}

TEST_CASE_FIX(vec_pop_f, vec_new_int10, vec_free)
{
    test_int_success(vec_pop_f(vec, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_last(vec), i[8]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(vec_pop_n_invalid_magic)
{
    vec_pop_n((vec_ct)&not_a_vector, 2);
}

TEST_CASE_FIX(vec_pop_n_empty_vector, vec_new_int, vec_free)
{
    test_int_error(vec_pop_n(vec, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_pop_n_nothing, vec_new_int1, vec_free)
{
    test_int_success(vec_pop_n(vec, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_pop_n, vec_new_int10, vec_free)
{
    test_int_success(vec_pop_n(vec, 2));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_last(vec), i[7]);
}

TEST_CASE_FIX(vec_pop_n_fewer_members, vec_new_int1, vec_free)
{
    test_int_error(vec_pop_n(vec, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_ABORT(vec_pop_en_invalid_magic)
{
    vec_pop_en((vec_ct)&not_a_vector, NULL, 2);
}

TEST_CASE_FIX(vec_pop_en_empty_vector, vec_new_int, vec_free)
{
    test_int_error(vec_pop_en(vec, NULL, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_pop_en_nothing, vec_new_int1, vec_free)
{
    test_int_success(vec_pop_en(vec, NULL, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_pop_en, vec_new_int10, vec_free)
{
    test_int_success(vec_pop_en(vec, j, 2));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_last(vec), i[7]);
    test_int_list_eq(j, &i[8], 2);
}

TEST_CASE_FIX(vec_pop_en_fewer_members, vec_new_int1, vec_free)
{
    test_int_error(vec_pop_en(vec, j, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_ABORT(vec_pop_fn_invalid_magic)
{
    vec_pop_fn((vec_ct)&not_a_vector, 2, NULL, NULL);
}

TEST_CASE_FIX(vec_pop_fn_empty_vector, vec_new_int, vec_free)
{
    test_int_error(vec_pop_fn(vec, 2, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_pop_fn_nothing, vec_new_int1, vec_free)
{
    test_int_success(vec_pop_fn(vec, 0, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_pop_fn, vec_new_int10, vec_free)
{
    test_int_success(vec_pop_fn(vec, 2, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_last(vec), i[7]);
    test_int_eq(count, 2);
}

TEST_CASE_FIX(vec_pop_fn_fewer_members, vec_new_int1, vec_free)
{
    test_int_error(vec_pop_fn(vec, 2, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_ABORT(vec_remove_invalid_magic)
{
    vec_remove((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_FIX_ABORT(vec_remove_invalid_elem, vec_new_int, vec_free)
{
    vec_remove(vec, NULL);
}

TEST_CASE_FIX(vec_remove_no_member, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_first(vec));
    test_int_error(vec_remove(vec, k - 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_remove_member_invalid_alignment, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_first(vec));
    vec_remove(vec, ((char *)k) + 1);
}

TEST_CASE_FIX(vec_remove, vec_new_int10, vec_free)
{
    test_int_success(vec_remove(vec, vec_first(vec)));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_first(vec), i[1]);
}

TEST_CASE_ABORT(vec_remove_f_invalid_magic)
{
    vec_remove_f((vec_ct)&not_a_vector, NULL, test_vec_dtor, &count);
}

TEST_CASE_FIX_ABORT(vec_remove_f_invalid_elem, vec_new_int, vec_free)
{
    vec_remove_f(vec, NULL, test_vec_dtor, &count);
}

TEST_CASE_FIX(vec_remove_f_no_member, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_first(vec));
    test_int_error(vec_remove_f(vec, k - 1, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_remove_f_member_invalid_alignment, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_first(vec));
    vec_remove_f(vec, ((char *)k) + 1, test_vec_dtor, &count);
}

TEST_CASE_FIX(vec_remove_f, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_f(vec, vec_first(vec), test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_first(vec), i[1]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(vec_remove_n_invalid_magic)
{
    vec_remove_n((vec_ct)&not_a_vector, NULL, 2);
}

TEST_CASE_FIX_ABORT(vec_remove_n_invalid_elem, vec_new_int, vec_free)
{
    vec_remove_n(vec, NULL, 2);
}

TEST_CASE_FIX(vec_remove_n_no_member, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_first(vec));
    test_int_error(vec_remove_n(vec, k - 1, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_remove_n_member_invalid_alignment, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_first(vec));
    vec_remove_n(vec, ((char *)k) + 1, 2);
}

TEST_CASE_FIX(vec_remove_n_nothing, vec_new_int1, vec_free)
{
    test_int_success(vec_remove_n(vec, vec_first(vec), 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_remove_n, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_n(vec, vec_first(vec), 2));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_first(vec), i[2]);
}

TEST_CASE_FIX(vec_remove_n_fewer_members, vec_new_int1, vec_free)
{
    test_int_error(vec_remove_n(vec, vec_first(vec), 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_ABORT(vec_remove_fn_invalid_magic)
{
    vec_remove_fn((vec_ct)&not_a_vector, NULL, 2, test_vec_dtor, &count);
}

TEST_CASE_FIX_ABORT(vec_remove_fn_invalid_elem, vec_new_int, vec_free)
{
    vec_remove_fn(vec, NULL, 2, test_vec_dtor, &count);
}

TEST_CASE_FIX(vec_remove_fn_no_member, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_first(vec));
    test_int_error(vec_remove_fn(vec, k - 1, 2, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX_ABORT(vec_remove_fn_member_invalid_alignment, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_first(vec));
    vec_remove_fn(vec, ((char *)k) + 1, 2, test_vec_dtor, &count);
}

TEST_CASE_FIX(vec_remove_fn_nothing, vec_new_int1, vec_free)
{
    test_int_success(vec_remove_fn(vec, vec_first(vec), 0, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_remove_fn, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_fn(vec, vec_first(vec), 2, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_first(vec), i[2]);
    test_int_eq(count, 2);
}

TEST_CASE_FIX(vec_remove_fn_fewer_members, vec_new_int1, vec_free)
{
    test_int_error(vec_remove_fn(vec, vec_first(vec), 2, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_ABORT(vec_remove_at_invalid_magic)
{
    vec_remove_at((vec_ct)&not_a_vector, 0);
}

TEST_CASE_FIX(vec_remove_at_oob_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at(vec, 0), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_oob_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at(vec, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at(vec, 0));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_first(vec), i[1]);
}

TEST_CASE_FIX(vec_remove_at_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at(vec, -1));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_last(vec), i[8]);
}

TEST_CASE_ABORT(vec_remove_at_e_invalid_magic)
{
    vec_remove_at_e((vec_ct)&not_a_vector, NULL, 0);
}

TEST_CASE_FIX(vec_remove_at_e_oob_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at_e(vec, NULL, 0), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_e_oob_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at_e(vec, NULL, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_e_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at_e(vec, j, 0));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_first(vec), i[1]);
    test_int_eq(j[0], i[0]);
}

TEST_CASE_FIX(vec_remove_at_e_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at_e(vec, j, -1));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_last(vec), i[8]);
    test_int_eq(j[0], i[9]);
}

TEST_CASE_ABORT(vec_remove_at_p_invalid_magic)
{
    vec_remove_at_p((vec_ct)&not_a_vector, 0);
}

TEST_CASE_FIX_ABORT(vec_remove_at_p_invalid_type, vec_new_int, vec_free)
{
    vec_remove_at_p(vec, 0);
}

TEST_CASE_FIX(vec_remove_at_p_oob_positive_index, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_remove_at_p(vec, 0), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_p_oob_negative_index, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_remove_at_p(vec, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_p_positive_index, vec_new_ptr10, vec_free)
{
    test_ptr_success(k = vec_remove_at_p(vec, 0));
    test_uint_eq(vec_size(vec), 9);
    test_ptr_eq(vec_first_p(vec), pi[1]);
    test_ptr_eq(k, pi[0]);
}

TEST_CASE_FIX(vec_remove_at_p_negative_index, vec_new_ptr10, vec_free)
{
    test_ptr_success(k = vec_remove_at_p(vec, -1));
    test_uint_eq(vec_size(vec), 9);
    test_ptr_eq(vec_last_p(vec), pi[8]);
    test_ptr_eq(k, pi[9]);
}

TEST_CASE_ABORT(vec_remove_at_f_invalid_magic)
{
    vec_remove_at_f((vec_ct)&not_a_vector, 0, test_vec_dtor, &count);
}

TEST_CASE_FIX(vec_remove_at_f_oob_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at_f(vec, 0, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_f_oob_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at_f(vec, -1, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_f_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at_f(vec, 0, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_first(vec), i[1]);
    test_int_eq(count, 1);
}

TEST_CASE_FIX(vec_remove_at_f_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at_f(vec, -1, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_last(vec), i[8]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(vec_remove_at_n_invalid_magic)
{
    vec_remove_at_n((vec_ct)&not_a_vector, 0, 2);
}

TEST_CASE_FIX(vec_remove_at_n_oob_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at_n(vec, 0, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_n_oob_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at_n(vec, -1, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_n_nothing_positive_index, vec_new_int1, vec_free)
{
    test_int_success(vec_remove_at_n(vec, 0, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_remove_at_n_nothing_negative_index, vec_new_int1, vec_free)
{
    test_int_success(vec_remove_at_n(vec, -1, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_remove_at_n_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at_n(vec, 0, 2));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_first(vec), i[2]);
}

TEST_CASE_FIX(vec_remove_at_n_fewer_members_positive_index, vec_new_int10, vec_free)
{
    test_int_error(vec_remove_at_n(vec, 9, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_n_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at_n(vec, -2, 2));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_last(vec), i[7]);
}

TEST_CASE_FIX(vec_remove_at_n_fewer_members_negative_index, vec_new_int10, vec_free)
{
    test_int_error(vec_remove_at_n(vec, -1, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_ABORT(vec_remove_at_en_invalid_magic)
{
    vec_remove_at_en((vec_ct)&not_a_vector, j, 0, 2);
}

TEST_CASE_FIX(vec_remove_at_en_oob_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at_en(vec, j, 0, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_en_oob_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at_en(vec, j, -1, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_en_nothing_positive_index, vec_new_int1, vec_free)
{
    test_int_success(vec_remove_at_en(vec, j, 0, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_remove_at_en_nothing_negative_index, vec_new_int1, vec_free)
{
    test_int_success(vec_remove_at_en(vec, j, -1, 0));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_remove_at_en_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at_en(vec, j, 0, 2));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_first(vec), i[2]);
    test_int_list_eq(j, i, 2);
}

TEST_CASE_FIX(vec_remove_at_en_fewer_members_positive_index, vec_new_int10, vec_free)
{
    test_int_error(vec_remove_at_en(vec, j, 9, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_en_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at_en(vec, j, -2, 2));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_last(vec), i[7]);
    test_int_list_eq(j, &i[8], 2);
}

TEST_CASE_FIX(vec_remove_at_en_fewer_members_negative_index, vec_new_int10, vec_free)
{
    test_int_error(vec_remove_at_en(vec, j, -1, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_ABORT(vec_remove_at_fn_invalid_magic)
{
    vec_remove_at_fn((vec_ct)&not_a_vector, 0, 2, test_vec_dtor, &count);
}

TEST_CASE_FIX(vec_remove_at_fn_oob_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at_fn(vec, 0, 2, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_fn_oob_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_remove_at_fn(vec, -1, 2, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_fn_nothing_positive_index, vec_new_int1, vec_free)
{
    test_int_success(vec_remove_at_fn(vec, 0, 0, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_remove_at_fn_nothing_negative_index, vec_new_int1, vec_free)
{
    test_int_success(vec_remove_at_fn(vec, -1, 0, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIX(vec_remove_at_fn_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at_fn(vec, 0, 2, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_first(vec), i[2]);
    test_int_eq(count, 2);
}

TEST_CASE_FIX(vec_remove_at_fn_fewer_members_positive_index, vec_new_int10, vec_free)
{
    test_int_error(vec_remove_at_fn(vec, 9, 2, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_remove_at_fn_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_remove_at_fn(vec, -2, 2, test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_last(vec), i[7]);
    test_int_eq(count, 2);
}

TEST_CASE_FIX(vec_remove_at_fn_fewer_members_negative_index, vec_new_int10, vec_free)
{
    test_int_error(vec_remove_at_fn(vec, -1, 2, test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

static bool test_vec_pred_int(vec_const_ct v, const void *elem, void *ctx)
{
    const int *i1 = elem, *i2 = ctx;

    return *i1 == *i2;
}

static bool test_vec_pred_ptr(vec_const_ct v, const void *elem, void *ctx)
{
    const int *const *p1 = elem, *const *p2 = ctx;

    return *p1 == *p2;
}

TEST_CASE_ABORT(vec_find_invalid_magic)
{
    vec_find((vec_ct)&not_a_vector, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_invalid_pred, vec_new_int, vec_free)
{
    vec_find((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX(vec_find_fail, vec_new_int, vec_free)
{
    test_ptr_error(vec_find(vec, test_vec_pred_int, (void *)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_find(vec, test_vec_pred_int, (void *)&i[2]));
    test_int_eq(vec_pos(vec, k), 2);
}

TEST_CASE_ABORT(vec_find_e_invalid_magic)
{
    vec_find_e((vec_ct)&not_a_vector, j, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_e_invalid_pred, vec_new_int, vec_free)
{
    vec_find_e(vec, NULL, NULL, NULL);
}

TEST_CASE_FIX(vec_find_e_fail, vec_new_int, vec_free)
{
    test_int_error(vec_find_e(vec, j, test_vec_pred_int, (void *)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_e, vec_new_int10, vec_free)
{
    test_int_success(vec_find_e(vec, j, test_vec_pred_int, (void *)&i[2]));
    test_int_eq(j[0], i[2]);
}

TEST_CASE_ABORT(vec_find_p_invalid_magic)
{
    vec_find_p((vec_ct)&not_a_vector, test_vec_pred_ptr, (void *)&pi[2]);
}

TEST_CASE_FIX_ABORT(vec_find_p_invalid_pred, vec_new_ptr, vec_free)
{
    vec_find_p((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_find_p_invalid_type, vec_new_int, vec_free)
{
    vec_find_p(vec, test_vec_pred_ptr, (void *)&pi[2]);
}

TEST_CASE_FIX(vec_find_p_fail, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_find_p(vec, test_vec_pred_ptr, (void *)&pi[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_p, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_find_p(vec, test_vec_pred_ptr, (void *)&pi[2]));
    test_ptr_eq(pk, pi[2]);
}

TEST_CASE_ABORT(vec_find_r_invalid_magic)
{
    vec_find_r((vec_ct)&not_a_vector, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_r_invalid_pred, vec_new_int, vec_free)
{
    vec_find_r((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX(vec_find_r_fail, vec_new_int, vec_free)
{
    test_ptr_error(vec_find_r(vec, test_vec_pred_int, (void *)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_r, vec_new_int10, vec_free)
{
    test_ptr_success(k = vec_find_r(vec, test_vec_pred_int, (void *)&i[2]));
    test_int_eq(vec_pos(vec, k), 7);
}

TEST_CASE_ABORT(vec_find_re_invalid_magic)
{
    vec_find_re((vec_ct)&not_a_vector, j, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_re_invalid_pred, vec_new_int, vec_free)
{
    vec_find_re(vec, NULL, NULL, NULL);
}

TEST_CASE_FIX(vec_find_re_fail, vec_new_int, vec_free)
{
    test_int_error(vec_find_re(vec, j, test_vec_pred_int, (void *)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_re, vec_new_int10, vec_free)
{
    test_int_success(vec_find_re(vec, j, test_vec_pred_int, (void *)&i[2]));
    test_int_eq(j[0], i[7]);
}

TEST_CASE_ABORT(vec_find_rp_invalid_magic)
{
    vec_find_rp((vec_ct)&not_a_vector, test_vec_pred_ptr, (void *)&pi[2]);
}

TEST_CASE_FIX_ABORT(vec_find_rp_invalid_pred, vec_new_ptr, vec_free)
{
    vec_find_rp((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_find_rp_invalid_type, vec_new_int, vec_free)
{
    vec_find_rp(vec, test_vec_pred_ptr, (void *)&pi[2]);
}

TEST_CASE_FIX(vec_find_rp_fail, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_find_rp(vec, test_vec_pred_ptr, (void *)&pi[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_rp, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_find_rp(vec, test_vec_pred_ptr, (void *)&pi[2]));
    test_ptr_eq(pk, pi[7]);
}

TEST_CASE_ABORT(vec_find_pos_invalid_magic)
{
    vec_find_pos((vec_ct)&not_a_vector, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_pos_invalid_pred, vec_new_int, vec_free)
{
    vec_find_pos(vec, NULL, NULL);
}

TEST_CASE_FIX(vec_find_pos_fail, vec_new_int, vec_free)
{
    test_int_error(vec_find_pos(vec, test_vec_pred_int, (void *)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_pos, vec_new_int10, vec_free)
{
    test_rc_success(vec_find_pos(vec, test_vec_pred_int, (void *)&i[2]), 2, -1);
}

TEST_CASE_ABORT(vec_find_pos_r_invalid_magic)
{
    vec_find_pos_r((vec_ct)&not_a_vector, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_pos_r_invalid_pred, vec_new_int, vec_free)
{
    vec_find_pos_r(vec, NULL, NULL);
}

TEST_CASE_FIX(vec_find_pos_r_fail, vec_new_int, vec_free)
{
    test_int_error(vec_find_pos_r(vec, test_vec_pred_int, (void *)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_pos_r, vec_new_int10, vec_free)
{
    test_rc_success(vec_find_pos_r(vec, test_vec_pred_int, (void *)&i[2]), 7, -1);
}

TEST_CASE_ABORT(vec_find_remove_invalid_magic)
{
    vec_find_remove((vec_ct)&not_a_vector, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_remove_invalid_pred, vec_new_int, vec_free)
{
    vec_find_remove(vec, NULL, NULL);
}

TEST_CASE_FIX(vec_find_remove_fail, vec_new_int, vec_free)
{
    test_int_error(vec_find_remove(vec, test_vec_pred_int, (void *)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_remove, vec_new_int10, vec_free)
{
    test_int_success(vec_find_remove(vec, test_vec_pred_int, (void *)&i[2]));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_at(vec, 2), i[3]);
}

TEST_CASE_ABORT(vec_find_remove_e_invalid_magic)
{
    vec_find_remove_e((vec_ct)&not_a_vector, NULL, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_remove_e_invalid_pred, vec_new_int, vec_free)
{
    vec_find_remove_e(vec, NULL, NULL, NULL);
}

TEST_CASE_FIX(vec_find_remove_e_fail, vec_new_int, vec_free)
{
    test_int_error(vec_find_remove_e(vec, NULL, test_vec_pred_int, (void *)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_remove_e, vec_new_int10, vec_free)
{
    test_int_success(vec_find_remove_e(vec, j, test_vec_pred_int, (void *)&i[2]));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_at(vec, 2), i[3]);
    test_int_eq(j[0], i[2]);
}

TEST_CASE_ABORT(vec_find_remove_p_invalid_magic)
{
    vec_find_remove_p((vec_ct)&not_a_vector, test_vec_pred_ptr, (void *)&pi[2]);
}

TEST_CASE_FIX_ABORT(vec_find_remove_p_invalid_pred, vec_new_ptr, vec_free)
{
    vec_find_remove_p(vec, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_find_remove_p_invalid_type, vec_new_int, vec_free)
{
    vec_find_remove_p(vec, test_vec_pred_ptr, (void *)&pi[2]);
}

TEST_CASE_FIX(vec_find_remove_p_fail, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_find_remove_p(vec, test_vec_pred_ptr, (void *)&pi[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_remove_p, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_find_remove_p(vec, test_vec_pred_ptr, (void *)&pi[2]));
    test_uint_eq(vec_size(vec), 9);
    test_ptr_eq(pk, pi[2]);
    test_ptr_eq(vec_at_p(vec, 2), pi[3]);
}

TEST_CASE_ABORT(vec_find_remove_f_invalid_magic)
{
    vec_find_remove_f((vec_ct)&not_a_vector, test_vec_pred_int, (void *)&i[2], test_vec_dtor, &count);
}

TEST_CASE_FIX_ABORT(vec_find_remove_f_invalid_pred, vec_new_int, vec_free)
{
    vec_find_remove_f(vec, NULL, NULL, NULL, NULL);
}

TEST_CASE_FIX(vec_find_remove_f_fail, vec_new_int, vec_free)
{
    test_int_error(vec_find_remove_f(vec, test_vec_pred_int, (void *)&i[2], test_vec_dtor, &count), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_remove_f, vec_new_int10, vec_free)
{
    test_int_success(vec_find_remove_f(vec, test_vec_pred_int, (void *)&i[2], test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_at(vec, 2), i[3]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(vec_find_remove_r_invalid_magic)
{
    vec_find_remove_r((vec_ct)&not_a_vector, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_remove_r_invalid_pred, vec_new_int, vec_free)
{
    vec_find_remove_r(vec, NULL, NULL);
}

TEST_CASE_FIX(vec_find_remove_r_fail, vec_new_int, vec_free)
{
    test_int_error(vec_find_remove_r(vec, test_vec_pred_int, (void *)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_remove_r, vec_new_int10, vec_free)
{
    test_int_success(vec_find_remove_r(vec, test_vec_pred_int, (void *)&i[2]));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_at(vec, 7), i[8]);
}

TEST_CASE_ABORT(vec_find_remove_re_invalid_magic)
{
    vec_find_remove_re((vec_ct)&not_a_vector, NULL, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_remove_re_invalid_pred, vec_new_int, vec_free)
{
    vec_find_remove_re(vec, NULL, NULL, NULL);
}

TEST_CASE_FIX(vec_find_remove_re_fail, vec_new_int, vec_free)
{
    test_int_error(vec_find_remove_re(vec, NULL, test_vec_pred_int, (void *)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_remove_re, vec_new_int10, vec_free)
{
    test_int_success(vec_find_remove_e(vec, j, test_vec_pred_int, (void *)&i[2]));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_at(vec, 7), i[8]);
    test_int_eq(j[0], i[7]);
}

TEST_CASE_ABORT(vec_find_remove_rp_invalid_magic)
{
    vec_find_remove_rp((vec_ct)&not_a_vector, test_vec_pred_ptr, (void *)&pi[2]);
}

TEST_CASE_FIX_ABORT(vec_find_remove_rp_invalid_pred, vec_new_ptr, vec_free)
{
    vec_find_remove_rp(vec, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_find_remove_rp_invalid_type, vec_new_int, vec_free)
{
    vec_find_remove_rp(vec, test_vec_pred_ptr, (void *)&pi[2]);
}

TEST_CASE_FIX(vec_find_remove_rp_fail, vec_new_ptr, vec_free)
{
    test_ptr_error(vec_find_remove_rp(vec, test_vec_pred_ptr, (void *)&pi[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_remove_rp, vec_new_ptr10, vec_free)
{
    test_ptr_success(pk = vec_find_remove_rp(vec, test_vec_pred_ptr, (void *)&pi[2]));
    test_uint_eq(vec_size(vec), 9);
    test_ptr_eq(pk, pi[7]);
    test_ptr_eq(vec_at_p(vec, 7), pi[8]);
}

TEST_CASE_ABORT(vec_find_remove_rf_invalid_magic)
{
    vec_find_remove_rf((vec_ct)&not_a_vector, test_vec_pred_int, (void *)&i[2], test_vec_dtor, &count);
}

TEST_CASE_FIX_ABORT(vec_find_remove_rf_invalid_pred, vec_new_int, vec_free)
{
    vec_find_remove_rf(vec, NULL, NULL, NULL, NULL);
}

TEST_CASE_FIX(vec_find_remove_rf_fail, vec_new_int, vec_free)
{
    test_int_error(vec_find_remove_rf(vec, test_vec_pred_int, (void *)&i[2], test_vec_dtor, &count), E_VEC_NOT_FOUND);
}

TEST_CASE_FIX(vec_find_remove_rf, vec_new_int10, vec_free)
{
    test_int_success(vec_find_remove_rf(vec, test_vec_pred_int, (void *)&i[2], test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int *)vec_at(vec, 7), i[8]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(vec_find_remove_all_invalid_magic)
{
    vec_find_remove_all((vec_ct)&not_a_vector, test_vec_pred_int, (void *)&i[2]);
}

TEST_CASE_FIX_ABORT(vec_find_remove_all_invalid_pred, vec_new_int, vec_free)
{
    vec_find_remove_all(vec, NULL, NULL);
}

TEST_CASE_FIX(vec_find_remove_all, vec_new_int10, vec_free)
{
    test_uint_eq(vec_find_remove_all(vec, test_vec_pred_int, (void *)&i[2]), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_at(vec, 2), i[3]);
    test_int_eq(*(int *)vec_at(vec, 6), i[8]);
}

TEST_CASE_ABORT(vec_find_remove_all_f_invalid_magic)
{
    vec_find_remove_all_f((vec_ct)&not_a_vector, test_vec_pred_int, (void *)&i[2], test_vec_dtor, &count);
}

TEST_CASE_FIX_ABORT(vec_find_remove_all_f_invalid_pred, vec_new_int, vec_free)
{
    vec_find_remove_all_f(vec, NULL, NULL, NULL, NULL);
}

TEST_CASE_FIX(vec_find_remove_all_f, vec_new_int10, vec_free)
{
    test_uint_eq(vec_find_remove_all_f(vec, test_vec_pred_int, (void *)&i[2], test_vec_dtor, &count), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int *)vec_at(vec, 2), i[3]);
    test_int_eq(*(int *)vec_at(vec, 6), i[8]);
    test_int_eq(count, 2);
}

TEST_CASE_ABORT(vec_swap_invalid_magic)
{
    vec_swap((vec_ct)&not_a_vector, 0, 9);
}

TEST_CASE_FIX(vec_swap_oob_positive_index, vec_new_int, vec_free)
{
    test_int_error(vec_swap(vec, 0, 9), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_swap_oob_negative_index, vec_new_int, vec_free)
{
    test_int_error(vec_swap(vec, -1, -10), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(vec_swap_positive_index, vec_new_int10, vec_free)
{
    test_int_success(vec_swap(vec, 0, 9));
    test_int_eq(*(int *)vec_first(vec), i[9]);
    test_int_eq(*(int *)vec_last(vec), i[0]);
}

TEST_CASE_FIX(vec_swap_negative_index, vec_new_int10, vec_free)
{
    test_int_success(vec_swap(vec, -1, -10));
    test_int_eq(*(int *)vec_first(vec), i[9]);
    test_int_eq(*(int *)vec_last(vec), i[0]);
}

TEST_CASE_ABORT(vec_get_buffer_invalid_magic)
{
    vec_get_buffer((vec_ct)&not_a_vector, NULL, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_get_buffer_invalid_buffer, vec_new_int, vec_free)
{
    vec_get_buffer(vec, NULL, NULL, NULL);
}

TEST_CASE_FIX(vec_get_buffer_no_capacity, vec_new_int, vec_free)
{
    void *buf;
    test_int_error(vec_get_buffer(vec, &buf, NULL, NULL), E_VEC_NO_BUFFER);
}

TEST_CASE_FIX(vec_get_buffer, vec_new_int10, vec_free)
{
    void *buf;
    size_t size, cap;

    test_int_success(vec_get_buffer(vec, &buf, &size, &cap));
    test_uint_eq(size, 10);
    test_uint_ge(cap, 10);
    test_uint_eq(vec_size(vec), 0);
    test_uint_eq(vec_capacity(vec), 0);
    test_int_list_eq((int *)buf, i, 10);

    test_void(free(buf));
}

TEST_CASE_ABORT(vec_truncate_invalid_magic)
{
    vec_truncate((vec_ct)&not_a_vector, 1);
}

TEST_CASE_FIX(vec_truncate, vec_new_int10, vec_free)
{
    test_uint_eq(vec_truncate(vec, 1), 9);
    test_uint_eq(vec_size(vec), 1);
    test_int_eq(*(int *)vec_first(vec), i[0]);
}

TEST_CASE_ABORT(vec_truncate_f_invalid_magic)
{
    vec_truncate_f((vec_ct)&not_a_vector, 1, test_vec_dtor, &count);
}

TEST_CASE_FIX(vec_truncate_f, vec_new_int10, vec_free)
{
    test_uint_eq(vec_truncate_f(vec, 1, test_vec_dtor, &count), 9);
    test_uint_eq(vec_size(vec), 1);
    test_int_eq(*(int *)vec_first(vec), i[0]);
    test_int_eq(count, 9);
}

TEST_CASE_ABORT(vec_set_capacity_invalid_magic)
{
    vec_set_capacity((vec_ct)&not_a_vector, 1);
}

TEST_CASE_FIX(vec_set_capacity_below_min_capacity, vec_new_int, vec_free)
{
    test_int_success(vec_set_capacity(vec, 1));
    test_uint_eq(vec_capacity(vec), vec_min_capacity(vec));
}

TEST_CASE_FIX(vec_set_capacity, vec_new_int, vec_free)
{
    test_int_success(vec_set_capacity(vec, 2 * vec_min_capacity(vec)));
    test_uint_eq(vec_capacity(vec), 2 * vec_min_capacity(vec));
}

TEST_CASE_FIX(vec_set_capacity_below_current_capacity, vec_new_int10, vec_free)
{
    test_int_success(vec_set_capacity(vec, 5));
    test_uint_eq(vec_capacity(vec), 5);
}

TEST_CASE_ABORT(vec_set_capacity_f_invalid_magic)
{
    vec_set_capacity_f((vec_ct)&not_a_vector, 1, test_vec_dtor, &count);
}

TEST_CASE_FIX(vec_set_capacity_f_below_min_capacity, vec_new_int, vec_free)
{
    test_int_success(vec_set_capacity_f(vec, 1, test_vec_dtor, &count));
    test_uint_eq(vec_capacity(vec), vec_min_capacity(vec));
    test_int_eq(count, 0);
}

TEST_CASE_FIX(vec_set_capacity_f, vec_new_int, vec_free)
{
    test_int_success(vec_set_capacity_f(vec, 2 * vec_min_capacity(vec), test_vec_dtor, &count));
    test_uint_eq(vec_capacity(vec), 2 * vec_min_capacity(vec));
    test_int_eq(count, 0);
}

TEST_CASE_FIX(vec_set_capacity_f_below_current_capacity, vec_new_int10, vec_free)
{
    test_int_success(vec_set_capacity_f(vec, 5, test_vec_dtor, &count));
    test_uint_eq(vec_capacity(vec), 5);
    test_int_eq(count, 5);
}

static int test_vec_fold(vec_const_ct v, size_t index, void *elem, void *ctx)
{
    int *i = elem, *sum = ctx;

    *sum = *sum * 10 + *i;

    return 0;
}

TEST_CASE_ABORT(vec_fold_invalid_magic)
{
    vec_fold((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_fold_invalid_fold, vec_new_int, vec_free)
{
    vec_fold(vec, NULL, NULL);
}

TEST_CASE_FIX(vec_fold, vec_new_int, vec_free)
{
    int i[] = { 1, 2, 3 };

    vec_push_en(vec, 3, i);
    test_int_success(vec_fold(vec, test_vec_fold, &count));
    test_int_eq(count, 123);
}

TEST_CASE_ABORT(vec_fold_r_invalid_magic)
{
    vec_fold_r((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_fold_r_invalid_fold, vec_new_int, vec_free)
{
    vec_fold_r(vec, NULL, NULL);
}

TEST_CASE_FIX(vec_fold_r, vec_new_int, vec_free)
{
    int i[] = { 1, 2, 3 };

    vec_push_en(vec, 3, i);
    test_int_success(vec_fold_r(vec, test_vec_fold, &count));
    test_int_eq(count, 321);
}

static int test_vec_sort(const void *elem1, const void *elem2, void *ctx)
{
    const int *i1 = elem1, *i2 = elem2;
    int *count = ctx;

    count[0]++;

    return *i1 < *i2 ? -1 : *i1 > *i2 ? 1 : 0;
}

TEST_CASE_ABORT(vec_sort_invalid_magic)
{
    vec_sort((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIX_ABORT(vec_sort_invalid_sort, vec_new_int, vec_free)
{
    vec_sort(vec, NULL, NULL);
}

TEST_CASE_FIX(vec_sort, vec_new_int, vec_free)
{
    int i[] = { 2, 1, 3 };

    vec_push_en(vec, 3, i);
    test_void(vec_sort(vec, test_vec_sort, &count));
    test_int_eq(count, 3);
    test_int_eq(*(int *)vec_at(vec, 0), 1);
    test_int_eq(*(int *)vec_at(vec, 1), 2);
    test_int_eq(*(int *)vec_at(vec, 2), 3);
}

int test_suite_con_vec(void *param)
{
    return error_pass_int(test_run_cases("vec",
        test_case(vec_new_invalid_elemsize),

        test_case(vec_elemsize_invalid_magic),
        test_case(vec_elemsize),

        test_case(vec_is_empty_invalid_magic),
        test_case(vec_is_empty_on_init),
        test_case(vec_is_not_empty_after_push),
        test_case(vec_is_empty_after_push_pop),

        test_case(vec_size_invalid_magic),
        test_case(vec_size_zero_on_init),
        test_case(vec_size_one_after_push),
        test_case(vec_size_zero_after_push_pop),

        test_case(vec_capacity_invalid_magic),
        test_case(vec_capacity_zero_on_init),
        test_case(vec_capacity_initial_capacity_after_push),
        test_case(vec_capacity_double_capacity_after_push_above_capacity),
        test_case(vec_capacity_initial_capacity_after_pop_below_threshold),

        test_case(vec_pos_invalid_magic),
        test_case(vec_pos_invalid_elem),
        test_case(vec_pos_no_member),
        test_case(vec_pos),
        test_case(vec_pos_no_member_before),
        test_case(vec_pos_no_member_after),
        test_case(vec_pos_member_invalid_alignment),

        test_case(vec_first_invalid_magic),
        test_case(vec_first_empty_vector),
        test_case(vec_first),
        test_case(vec_first_p_invalid_magic),
        test_case(vec_first_p_invalid_type),
        test_case(vec_first_p_empty_vector),
        test_case(vec_first_p),
        test_case(vec_last_invalid_magic),
        test_case(vec_last_empty_vector),
        test_case(vec_last),
        test_case(vec_last_p_invalid_magic),
        test_case(vec_last_p_invalid_type),
        test_case(vec_last_p_empty_vector),
        test_case(vec_last_p),

        test_case(vec_at_invalid_magic),
        test_case(vec_at_empty_vector_positive_index),
        test_case(vec_at_empty_vector_negative_index),
        test_case(vec_at_positive_index),
        test_case(vec_at_negative_index),
        test_case(vec_at_p_invalid_magic),
        test_case(vec_at_p_invalid_type),
        test_case(vec_at_p_empty_vector_positive_index),
        test_case(vec_at_p_empty_vector_negative_index),
        test_case(vec_at_p_positive_index),
        test_case(vec_at_p_negative_index),

        test_case(vec_get_first_invalid_magic),
        test_case(vec_get_first_empty_vector),
        test_case(vec_get_first),
        test_case(vec_get_last_invalid_magic),
        test_case(vec_get_last_empty_vector),
        test_case(vec_get_last),
        test_case(vec_get_invalid_magic),
        test_case(vec_get_empty_vector_positive_index),
        test_case(vec_get_empty_vector_negative_index),
        test_case(vec_get_positive_index),
        test_case(vec_get_negative_index),
        test_case(vec_get_n_invalid_magic),
        test_case(vec_get_n_empty_vector_positive_index),
        test_case(vec_get_n_empty_vector_negative_index),
        test_case(vec_get_n_nothing_positive_index),
        test_case(vec_get_n_nothing_negative_index),
        test_case(vec_get_n_positive_index),
        test_case(vec_get_n_negative_index),

        test_case(vec_push_invalid_magic),
        test_case(vec_push),
        test_case(vec_push_e_invalid_magic),
        test_case(vec_push_e),
        test_case(vec_push_p_invalid_magic),
        test_case(vec_push_p_invalid_type),
        test_case(vec_push_p),
        test_case(vec_push_n_invalid_magic),
        test_case(vec_push_n_nothing),
        test_case(vec_push_n),
        test_case(vec_push_en_invalid_magic),
        test_case(vec_push_en_nothing),
        test_case(vec_push_en),
        test_case(vec_push_a_invalid_magic),
        test_case(vec_push_a_nothing),
        test_case(vec_push_a),
        test_case(vec_push_ap_invalid_magic),
        test_case(vec_push_ap_invalid_type),
        test_case(vec_push_ap_nothing),
        test_case(vec_push_ap),

        test_case(vec_insert_invalid_magic),
        test_case(vec_insert_oob_positive_index),
        test_case(vec_insert_oob_negative_index),
        test_case(vec_insert_front_positive_index),
        test_case(vec_insert_front_negative_index),
        test_case(vec_insert_back_positive_index),
        test_case(vec_insert_back_negative_index),

        test_case(vec_insert_e_invalid_magic),
        test_case(vec_insert_e_oob_positive_index),
        test_case(vec_insert_e_oob_negative_index),
        test_case(vec_insert_e_front_positive_index),
        test_case(vec_insert_e_front_negative_index),
        test_case(vec_insert_e_back_positive_index),
        test_case(vec_insert_e_back_negative_index),

        test_case(vec_insert_p_invalid_magic),
        test_case(vec_insert_p_invalid_type),
        test_case(vec_insert_p_oob_positive_index),
        test_case(vec_insert_p_oob_negative_index),
        test_case(vec_insert_p_front_positive_index),
        test_case(vec_insert_p_front_negative_index),
        test_case(vec_insert_p_back_positive_index),
        test_case(vec_insert_p_back_negative_index),

        test_case(vec_insert_n_invalid_magic),
        test_case(vec_insert_n_oob_positive_index),
        test_case(vec_insert_n_oob_negative_index),
        test_case(vec_insert_n_nothing_positive_index),
        test_case(vec_insert_n_nothing_negative_index),
        test_case(vec_insert_n_front_positive_index),
        test_case(vec_insert_n_front_negative_index),
        test_case(vec_insert_n_back_positive_index),
        test_case(vec_insert_n_back_negative_index),

        test_case(vec_insert_en_invalid_magic),
        test_case(vec_insert_en_oob_positive_index),
        test_case(vec_insert_en_oob_negative_index),
        test_case(vec_insert_en_nothing_positive_index),
        test_case(vec_insert_en_nothing_negative_index),
        test_case(vec_insert_en_front_positive_index),
        test_case(vec_insert_en_front_negative_index),
        test_case(vec_insert_en_back_positive_index),
        test_case(vec_insert_en_back_negative_index),

        test_case(vec_insert_before_invalid_magic),
        test_case(vec_insert_before_invalid_elem),
        test_case(vec_insert_before_no_member),
        test_case(vec_insert_before_member_invalid_alignment),
        test_case(vec_insert_before),
        test_case(vec_insert_before_e_invalid_magic),
        test_case(vec_insert_before_e_invalid_elem),
        test_case(vec_insert_before_e_no_member),
        test_case(vec_insert_before_e_member_invalid_alignment),
        test_case(vec_insert_before_e),
        test_case(vec_insert_before_p_invalid_magic),
        test_case(vec_insert_before_p_invalid_elem),
        test_case(vec_insert_before_p_invalid_type),
        test_case(vec_insert_before_p_no_member),
        test_case(vec_insert_before_p_member_invalid_alignment),
        test_case(vec_insert_before_p),
        test_case(vec_insert_before_n_invalid_magic),
        test_case(vec_insert_before_n_invalid_elem),
        test_case(vec_insert_before_n_no_member),
        test_case(vec_insert_before_n_member_invalid_alignment),
        test_case(vec_insert_before_n_nothing),
        test_case(vec_insert_before_n),
        test_case(vec_insert_before_en_invalid_magic),
        test_case(vec_insert_before_en_invalid_elem),
        test_case(vec_insert_before_en_no_member),
        test_case(vec_insert_before_en_member_invalid_alignment),
        test_case(vec_insert_before_en_nothing),
        test_case(vec_insert_before_en),

        test_case(vec_insert_after_invalid_magic),
        test_case(vec_insert_after_invalid_elem),
        test_case(vec_insert_after_no_member),
        test_case(vec_insert_after_member_invalid_alignment),
        test_case(vec_insert_after),
        test_case(vec_insert_after_e_invalid_magic),
        test_case(vec_insert_after_e_invalid_elem),
        test_case(vec_insert_after_e_no_member),
        test_case(vec_insert_after_e_member_invalid_alignment),
        test_case(vec_insert_after_e),
        test_case(vec_insert_after_p_invalid_magic),
        test_case(vec_insert_after_p_invalid_elem),
        test_case(vec_insert_after_p_invalid_type),
        test_case(vec_insert_after_p_no_member),
        test_case(vec_insert_after_p_member_invalid_alignment),
        test_case(vec_insert_after_p),
        test_case(vec_insert_after_n_invalid_magic),
        test_case(vec_insert_after_n_invalid_elem),
        test_case(vec_insert_after_n_no_member),
        test_case(vec_insert_after_n_member_invalid_alignment),
        test_case(vec_insert_after_n_nothing),
        test_case(vec_insert_after_n),
        test_case(vec_insert_after_en_invalid_magic),
        test_case(vec_insert_after_en_invalid_elem),
        test_case(vec_insert_after_en_no_member),
        test_case(vec_insert_after_en_member_invalid_alignment),
        test_case(vec_insert_after_en_nothing),
        test_case(vec_insert_after_en),

        test_case(vec_set_invalid_magic),
        test_case(vec_set_oob_positive_index),
        test_case(vec_set_oob_negative_index),
        test_case(vec_set_front_positive_index),
        test_case(vec_set_front_negative_index),
        test_case(vec_set_back_positive_index),
        test_case(vec_set_back_negative_index),
        test_case(vec_set_f_invalid_magic),
        test_case(vec_set_f_oob_positive_index),
        test_case(vec_set_f_oob_negative_index),
        test_case(vec_set_f_front_positive_index),
        test_case(vec_set_f_front_negative_index),
        test_case(vec_set_f_back_positive_index),
        test_case(vec_set_f_back_negative_index),

        test_case(vec_pop_invalid_magic),
        test_case(vec_pop_empty_vector),
        test_case(vec_pop),
        test_case(vec_pop_e_invalid_magic),
        test_case(vec_pop_e_empty_vector),
        test_case(vec_pop_e),
        test_case(vec_pop_p_invalid_magic),
        test_case(vec_pop_p_invalid_type),
        test_case(vec_pop_p_empty_vector),
        test_case(vec_pop_p),
        test_case(vec_pop_f_invalid_magic),
        test_case(vec_pop_f_empty_vector),
        test_case(vec_pop_f),
        test_case(vec_pop_n_invalid_magic),
        test_case(vec_pop_n_empty_vector),
        test_case(vec_pop_n_nothing),
        test_case(vec_pop_n),
        test_case(vec_pop_n_fewer_members),
        test_case(vec_pop_en_invalid_magic),
        test_case(vec_pop_en_empty_vector),
        test_case(vec_pop_en_nothing),
        test_case(vec_pop_en),
        test_case(vec_pop_en_fewer_members),
        test_case(vec_pop_fn_invalid_magic),
        test_case(vec_pop_fn_empty_vector),
        test_case(vec_pop_fn_nothing),
        test_case(vec_pop_fn),
        test_case(vec_pop_fn_fewer_members),

        test_case(vec_remove_invalid_magic),
        test_case(vec_remove_invalid_elem),
        test_case(vec_remove_no_member),
        test_case(vec_remove_member_invalid_alignment),
        test_case(vec_remove),
        test_case(vec_remove_f_invalid_magic),
        test_case(vec_remove_f_invalid_elem),
        test_case(vec_remove_f_no_member),
        test_case(vec_remove_f_member_invalid_alignment),
        test_case(vec_remove_f),

        test_case(vec_remove_n_invalid_magic),
        test_case(vec_remove_n_invalid_elem),
        test_case(vec_remove_n_no_member),
        test_case(vec_remove_n_member_invalid_alignment),
        test_case(vec_remove_n_nothing),
        test_case(vec_remove_n),
        test_case(vec_remove_n_fewer_members),
        test_case(vec_remove_fn_invalid_magic),
        test_case(vec_remove_fn_invalid_elem),
        test_case(vec_remove_fn_no_member),
        test_case(vec_remove_fn_member_invalid_alignment),
        test_case(vec_remove_fn_nothing),
        test_case(vec_remove_fn),
        test_case(vec_remove_fn_fewer_members),

        test_case(vec_remove_at_invalid_magic),
        test_case(vec_remove_at_oob_positive_index),
        test_case(vec_remove_at_oob_negative_index),
        test_case(vec_remove_at_positive_index),
        test_case(vec_remove_at_negative_index),

        test_case(vec_remove_at_e_invalid_magic),
        test_case(vec_remove_at_e_oob_positive_index),
        test_case(vec_remove_at_e_oob_negative_index),
        test_case(vec_remove_at_e_positive_index),
        test_case(vec_remove_at_e_negative_index),

        test_case(vec_remove_at_p_invalid_magic),
        test_case(vec_remove_at_p_invalid_type),
        test_case(vec_remove_at_p_oob_positive_index),
        test_case(vec_remove_at_p_oob_negative_index),
        test_case(vec_remove_at_p_positive_index),
        test_case(vec_remove_at_p_negative_index),

        test_case(vec_remove_at_f_invalid_magic),
        test_case(vec_remove_at_f_oob_positive_index),
        test_case(vec_remove_at_f_oob_negative_index),
        test_case(vec_remove_at_f_positive_index),
        test_case(vec_remove_at_f_negative_index),

        test_case(vec_remove_at_n_invalid_magic),
        test_case(vec_remove_at_n_oob_positive_index),
        test_case(vec_remove_at_n_oob_negative_index),
        test_case(vec_remove_at_n_nothing_positive_index),
        test_case(vec_remove_at_n_nothing_negative_index),
        test_case(vec_remove_at_n_positive_index),
        test_case(vec_remove_at_n_fewer_members_positive_index),
        test_case(vec_remove_at_n_negative_index),
        test_case(vec_remove_at_n_fewer_members_negative_index),

        test_case(vec_remove_at_en_invalid_magic),
        test_case(vec_remove_at_en_oob_positive_index),
        test_case(vec_remove_at_en_oob_negative_index),
        test_case(vec_remove_at_en_nothing_positive_index),
        test_case(vec_remove_at_en_nothing_negative_index),
        test_case(vec_remove_at_en_positive_index),
        test_case(vec_remove_at_en_fewer_members_positive_index),
        test_case(vec_remove_at_en_negative_index),
        test_case(vec_remove_at_en_fewer_members_negative_index),

        test_case(vec_remove_at_fn_invalid_magic),
        test_case(vec_remove_at_fn_oob_positive_index),
        test_case(vec_remove_at_fn_oob_negative_index),
        test_case(vec_remove_at_fn_nothing_positive_index),
        test_case(vec_remove_at_fn_nothing_negative_index),
        test_case(vec_remove_at_fn_positive_index),
        test_case(vec_remove_at_fn_fewer_members_positive_index),
        test_case(vec_remove_at_fn_negative_index),
        test_case(vec_remove_at_fn_fewer_members_negative_index),

        test_case(vec_find_invalid_magic),
        test_case(vec_find_invalid_pred),
        test_case(vec_find_fail),
        test_case(vec_find),
        test_case(vec_find_e_invalid_magic),
        test_case(vec_find_e_invalid_pred),
        test_case(vec_find_e_fail),
        test_case(vec_find_e),
        test_case(vec_find_p_invalid_magic),
        test_case(vec_find_p_invalid_pred),
        test_case(vec_find_p_invalid_type),
        test_case(vec_find_p_fail),
        test_case(vec_find_p),
        test_case(vec_find_r_invalid_magic),
        test_case(vec_find_r_invalid_pred),
        test_case(vec_find_r_fail),
        test_case(vec_find_r),
        test_case(vec_find_re_invalid_magic),
        test_case(vec_find_re_invalid_pred),
        test_case(vec_find_re_fail),
        test_case(vec_find_re),
        test_case(vec_find_rp_invalid_magic),
        test_case(vec_find_rp_invalid_pred),
        test_case(vec_find_rp_invalid_type),
        test_case(vec_find_rp_fail),
        test_case(vec_find_rp),

        test_case(vec_find_pos_invalid_magic),
        test_case(vec_find_pos_invalid_pred),
        test_case(vec_find_pos_fail),
        test_case(vec_find_pos),
        test_case(vec_find_pos_r_invalid_magic),
        test_case(vec_find_pos_r_invalid_pred),
        test_case(vec_find_pos_r_fail),
        test_case(vec_find_pos_r),

        test_case(vec_find_remove_invalid_magic),
        test_case(vec_find_remove_invalid_pred),
        test_case(vec_find_remove_fail),
        test_case(vec_find_remove),
        test_case(vec_find_remove_e_invalid_magic),
        test_case(vec_find_remove_e_invalid_pred),
        test_case(vec_find_remove_e_fail),
        test_case(vec_find_remove_e),
        test_case(vec_find_remove_p_invalid_magic),
        test_case(vec_find_remove_p_invalid_pred),
        test_case(vec_find_remove_p_invalid_type),
        test_case(vec_find_remove_p_fail),
        test_case(vec_find_remove_p),
        test_case(vec_find_remove_f_invalid_magic),
        test_case(vec_find_remove_f_invalid_pred),
        test_case(vec_find_remove_f_fail),
        test_case(vec_find_remove_f),
        test_case(vec_find_remove_r_invalid_magic),
        test_case(vec_find_remove_r_invalid_pred),
        test_case(vec_find_remove_r_fail),
        test_case(vec_find_remove_r),
        test_case(vec_find_remove_re_invalid_magic),
        test_case(vec_find_remove_re_invalid_pred),
        test_case(vec_find_remove_re_fail),
        test_case(vec_find_remove_re),
        test_case(vec_find_remove_rp_invalid_magic),
        test_case(vec_find_remove_rp_invalid_pred),
        test_case(vec_find_remove_rp_invalid_type),
        test_case(vec_find_remove_rp_fail),
        test_case(vec_find_remove_rp),
        test_case(vec_find_remove_rf_invalid_magic),
        test_case(vec_find_remove_rf_invalid_pred),
        test_case(vec_find_remove_rf_fail),
        test_case(vec_find_remove_rf),
        test_case(vec_find_remove_all_invalid_magic),
        test_case(vec_find_remove_all_invalid_pred),
        test_case(vec_find_remove_all),
        test_case(vec_find_remove_all_f_invalid_magic),
        test_case(vec_find_remove_all_f_invalid_pred),
        test_case(vec_find_remove_all_f),

        test_case(vec_swap_invalid_magic),
        test_case(vec_swap_oob_positive_index),
        test_case(vec_swap_oob_negative_index),
        test_case(vec_swap_positive_index),
        test_case(vec_swap_negative_index),

        test_case(vec_get_buffer_invalid_magic),
        test_case(vec_get_buffer_invalid_buffer),
        test_case(vec_get_buffer_no_capacity),
        test_case(vec_get_buffer),

        test_case(vec_truncate_invalid_magic),
        test_case(vec_truncate),
        test_case(vec_truncate_f_invalid_magic),
        test_case(vec_truncate_f),

        test_case(vec_set_capacity_invalid_magic),
        test_case(vec_set_capacity_below_min_capacity),
        test_case(vec_set_capacity),
        test_case(vec_set_capacity_below_current_capacity),
        test_case(vec_set_capacity_f_invalid_magic),
        test_case(vec_set_capacity_f_below_min_capacity),
        test_case(vec_set_capacity_f),
        test_case(vec_set_capacity_f_below_current_capacity),

        test_case(vec_fold_invalid_magic),
        test_case(vec_fold_invalid_fold),
        test_case(vec_fold),
        test_case(vec_fold_r_invalid_magic),
        test_case(vec_fold_r_invalid_fold),
        test_case(vec_fold_r),

        test_case(vec_sort_invalid_magic),
        test_case(vec_sort_invalid_sort),
        test_case(vec_sort),

        NULL
    ));
}
