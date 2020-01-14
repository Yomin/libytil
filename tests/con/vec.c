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

#include "vec.h"
#include <ytil/test/test.h>
#include <ytil/con/vec.h>

#define vec TEST_STATE

static const struct not_a_vector
{
    int foo;
} not_a_vector = { 123 };

static const int   i[] = { 1, 2, 33, 4, 5, 6, 7, 33, 9, 10 };
static const int *pi[] = { &i[0], &i[1], &i[2], &i[3], &i[4], &i[5], &i[6], &i[2], &i[8], &i[9] };
static int j[10], *k, **pk, count;


TEST_SETUP(vec_int)
{
    vec = vec_new(5, sizeof(int));
    memset(j, 0, sizeof(j));
    count = 0;
}

TEST_SETUP(vec_int1)
{
    vec = vec_new(5, sizeof(int));
    vec_push_e(vec, i);
    memset(j, 0, sizeof(j));
    count = 0;
}

TEST_SETUP(vec_int10)
{
    vec = vec_new(5, sizeof(int));
    vec_push_en(vec, 10, i);
    memset(j, 0, sizeof(j));
    count = 0;
}

TEST_SETUP(vec_ptr)
{
    vec = vec_new(5, sizeof(int*));
}

TEST_SETUP(vec_ptr1)
{
    vec = vec_new(5, sizeof(int*));
    vec_push_p(vec, pi[0]);
}

TEST_SETUP(vec_ptr10)
{
    vec = vec_new(5, sizeof(int*));
    vec_push_en(vec, 10, pi);
}

TEST_TEARDOWN(vec)
{
    vec_free(vec);
}

TEST_CASE(vec_new_invalid_capacity)
{
    test_ptr_error(vec_new(0, 0), E_VEC_INVALID_ELEMSIZE);
}

TEST_CASE_ABORT(vec_elemsize_invalid_magic)
{
    vec_elemsize((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIXTURE(vec_elemsize, vec_int, vec)
{
    test_uint_eq(vec_elemsize(vec), sizeof(int));
}

TEST_CASE_ABORT(vec_is_empty_invalid_magic)
{
    vec_is_empty((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIXTURE(vec_is_empty_on_init, vec_int, vec)
{
    test_true(vec_is_empty(vec));
}

TEST_CASE_FIXTURE(vec_is_not_empty_after_push, vec_int, vec)
{
    vec_push(vec);
    test_false(vec_is_empty(vec));
}

TEST_CASE_FIXTURE(vec_is_empty_after_push_pop, vec_int, vec)
{
    vec_push(vec);
    vec_pop(vec);
    test_true(vec_is_empty(vec));
}

TEST_CASE_ABORT(vec_size_invalid_magic)
{
    vec_size((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIXTURE(vec_size_zero_on_init, vec_int, vec)
{
    test_uint_eq(vec_size(vec), 0);
}

TEST_CASE_FIXTURE(vec_size_one_after_push, vec_int, vec)
{
    vec_push(vec);
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_FIXTURE(vec_size_zero_after_push_pop, vec_int, vec)
{
    vec_push(vec);
    vec_pop(vec);
    test_uint_eq(vec_size(vec), 0);
}

TEST_CASE_ABORT(vec_capacity_invalid_magic)
{
    vec_capacity((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIXTURE(vec_capacity_zero_on_init, vec_int, vec)
{
    test_uint_eq(vec_capacity(vec), 0);
}

TEST_CASE_FIXTURE(vec_capacity_initial_capacity_after_push, vec_int, vec)
{
    vec_push(vec);
    test_uint_gt(vec_capacity(vec), 0);
}

TEST_CASE_FIXTURE(vec_capacity_double_capacity_after_push_above_capacity, vec_int, vec)
{
    size_t cap;
    
    vec_push(vec);
    cap = vec_capacity(vec);
    vec_push_n(vec, cap);
    
    test_uint_eq(vec_capacity(vec), 2*cap);
}

TEST_CASE_FIXTURE(vec_capacity_initial_capacity_after_pop_below_threshold, vec_int, vec)
{
    size_t cap;
    
    vec_push(vec);
    cap = vec_capacity(vec);
    vec_push_n(vec, cap);
    vec_pop_n(vec, cap);
    
    test_uint_eq(vec_capacity(vec), cap);
}

TEST_CASE_ABORT(vec_pos_invalid_magic)
{
    vec_pos((vec_const_ct)&not_a_vector, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_pos_invalid_elem, vec_int, vec)
{
    vec_pos(vec, NULL);
}

TEST_CASE_FIXTURE(vec_pos_no_member, vec_int, vec)
{
    test_int_error(vec_pos(vec, &not_a_vector), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_pos, vec_int10, vec)
{
    int *i = vec_first(vec);
    
    test_rc_success(vec_pos(vec, i+5), 5);
}

TEST_CASE_FIXTURE(vec_pos_no_member_before, vec_int10, vec)
{
    int *i = vec_first(vec);
    
    test_int_error(vec_pos(vec, i-1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_pos_no_member_after, vec_int10, vec)
{
    int *i = vec_last(vec);
    
    test_int_error(vec_pos(vec, i+1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_pos_member_invalid_alignment, vec_int10, vec)
{
    int *i = vec_first(vec);
    
    test_int_error(vec_pos(vec, ((char*)i)+1), E_VEC_UNALIGNED);
}

TEST_CASE_ABORT(vec_first_invalid_magic)
{
    vec_first((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIXTURE(vec_first_empty_vector, vec_int, vec)
{
    test_ptr_error(vec_first(vec), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_first, vec_int10, vec)
{
    test_int_eq(*(int*)vec_first(vec), i[0]);
}

TEST_CASE_ABORT(vec_first_p_invalid_magic)
{
    vec_first_p((vec_const_ct)&not_a_vector);
}

TEST_CASE_ABORT_FIXTURE(vec_first_p_invalid_type, vec_int, vec)
{
    vec_first_p(vec);
}

TEST_CASE_FIXTURE(vec_first_p_empty_vector, vec_ptr, vec)
{
    test_ptr_error(vec_first_p(vec), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_first_p, vec_ptr10, vec)
{
    test_ptr_eq(vec_first_p(vec), pi[0]);
}

TEST_CASE_ABORT(vec_last_invalid_magic)
{
    vec_last((vec_const_ct)&not_a_vector);
}

TEST_CASE_FIXTURE(vec_last_empty_vector, vec_int, vec)
{
    test_ptr_error(vec_last(vec), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_last, vec_int10, vec)
{
    test_int_eq(*(int*)vec_last(vec), i[9]);
}

TEST_CASE_ABORT(vec_last_p_invalid_magic)
{
    vec_last_p((vec_const_ct)&not_a_vector);
}

TEST_CASE_ABORT_FIXTURE(vec_last_p_invalid_type, vec_int, vec)
{
    vec_last_p(vec);
}

TEST_CASE_FIXTURE(vec_last_p_empty_vector, vec_ptr, vec)
{
    test_ptr_error(vec_last_p(vec), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_last_p, vec_ptr10, vec)
{
    test_ptr_eq(vec_last_p(vec), pi[9]);
}

TEST_CASE_ABORT(vec_at_invalid_magic)
{
    vec_at((vec_const_ct)&not_a_vector, 0);
}

TEST_CASE_FIXTURE(vec_at_empty_vector_positive_index, vec_int, vec)
{
    test_ptr_error(vec_at(vec, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_at_empty_vector_negative_index, vec_int, vec)
{
    test_ptr_error(vec_at(vec, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_at_positive_index, vec_int10, vec)
{
    test_int_eq(*(int*)vec_at(vec, 4), i[4]);
}

TEST_CASE_FIXTURE(vec_at_negative_index, vec_int10, vec)
{
    test_int_eq(*(int*)vec_at(vec, -4), i[6]);
}

TEST_CASE_ABORT(vec_at_p_invalid_magic)
{
    vec_at_p((vec_const_ct)&not_a_vector, 0);
}

TEST_CASE_ABORT_FIXTURE(vec_at_p_invalid_type, vec_int, vec)
{
    vec_at_p(vec, 0);
}

TEST_CASE_FIXTURE(vec_at_p_empty_vector_positive_index, vec_ptr, vec)
{
    test_ptr_error(vec_at_p(vec, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_at_p_empty_vector_negative_index, vec_ptr, vec)
{
    test_ptr_error(vec_at_p(vec, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_at_p_positive_index, vec_ptr10, vec)
{
    test_ptr_eq(vec_at_p(vec, 4), pi[4]);
}

TEST_CASE_FIXTURE(vec_at_p_negative_index, vec_ptr10, vec)
{
    test_ptr_eq(vec_at_p(vec, -4), pi[6]);
}

TEST_CASE_ABORT(vec_get_first_invalid_magic)
{
    vec_get_first((vec_const_ct)&not_a_vector, NULL);
}

TEST_CASE_FIXTURE(vec_get_first_empty_vector, vec_int, vec)
{
    test_int_error(vec_get_first(vec, j), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_get_first, vec_int10, vec)
{
    test_int_success(vec_get_first(vec, j));
    test_int_eq(j[0], i[0]);
}

TEST_CASE_ABORT(vec_get_last_invalid_magic)
{
    vec_get_last((vec_const_ct)&not_a_vector, NULL);
}

TEST_CASE_FIXTURE(vec_get_last_empty_vector, vec_int, vec)
{
    test_int_error(vec_get_last(vec, j), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_get_last, vec_int10, vec)
{
    test_int_success(vec_get_last(vec, j));
    test_int_eq(j[0], i[9]);
}

TEST_CASE_ABORT(vec_get_invalid_magic)
{
    vec_get((vec_const_ct)&not_a_vector, NULL, 0);
}

TEST_CASE_FIXTURE(vec_get_empty_vector_positive_index, vec_int, vec)
{
    test_int_error(vec_get(vec, j, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_get_empty_vector_negative_index, vec_int, vec)
{
    test_int_error(vec_get(vec, j, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_get_positive_index, vec_int10, vec)
{
    test_int_success(vec_get(vec, j, 4));
    test_int_eq(j[0], i[4]);
}

TEST_CASE_FIXTURE(vec_get_negative_index, vec_int10, vec)
{
    test_int_success(vec_get(vec, j, -4));
    test_int_eq(j[0], i[6]);
}

TEST_CASE_ABORT(vec_get_n_invalid_magic)
{
    vec_get_n((vec_const_ct)&not_a_vector, NULL, 0, 1);
}

TEST_CASE_FIXTURE(vec_get_n_empty_vector_positive_index, vec_int, vec)
{
    test_int_error(vec_get_n(vec, j, 1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_get_n_empty_vector_negative_index, vec_int, vec)
{
    test_int_error(vec_get_n(vec, j, -1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_get_n_nothing_positive_index, vec_int1, vec)
{
    test_int_error(vec_get_n(vec, j, 0, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_get_n_nothing_negative_index, vec_int1, vec)
{
    test_int_error(vec_get_n(vec, j, -1, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_get_n_positive_index, vec_int10, vec)
{
    test_rc_success(vec_get_n(vec, j, 4, 2), 2);
    test_int_list(j, i+4, 2);
}

TEST_CASE_FIXTURE(vec_get_n_negative_index, vec_int10, vec)
{
    test_rc_success(vec_get_n(vec, j, -4, 2), 2);
    test_int_list(j, i+6, 2);
}

TEST_CASE_ABORT(vec_push_invalid_magic)
{
    vec_push((vec_ct)&not_a_vector);
}

TEST_CASE_FIXTURE(vec_push, vec_int, vec)
{
    test_ptr_success(vec_push(vec));
    test_uint_eq(vec_size(vec), 1);
}

TEST_CASE_ABORT(vec_push_e_invalid_magic)
{
    vec_push_e((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_FIXTURE(vec_push_e, vec_int, vec)
{
    test_ptr_success(vec_push_e(vec, i));
    test_uint_eq(vec_size(vec), 1);
    test_int_eq(*(int*)vec_at(vec, 0), i[0]);
}

TEST_CASE_ABORT(vec_push_p_invalid_magic)
{
    vec_push_p((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_push_p_invalid_type, vec_int, vec)
{
    vec_push_p(vec, NULL);
}

TEST_CASE_FIXTURE(vec_push_p, vec_ptr, vec)
{
    test_ptr_success(vec_push_p(vec, pi[0]));
    test_uint_eq(vec_size(vec), 1);
    test_ptr_eq(vec_at_p(vec, 0), pi[0]);
}

TEST_CASE_ABORT(vec_push_n_invalid_magic)
{
    vec_push_n((vec_ct)&not_a_vector, 2);
}

TEST_CASE_FIXTURE(vec_push_n_nothing, vec_int1, vec)
{
    test_ptr_error(vec_push_n(vec, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_push_n, vec_int, vec)
{
    test_ptr_success(vec_push_n(vec, 2));
    test_uint_eq(vec_size(vec), 2);
}

TEST_CASE_ABORT(vec_push_en_invalid_magic)
{
    vec_push_en((vec_ct)&not_a_vector, 2, NULL);
}

TEST_CASE_FIXTURE(vec_push_en_nothing, vec_int1, vec)
{
    test_ptr_error(vec_push_en(vec, 0, NULL), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_push_en, vec_int, vec)
{
    test_ptr_success(vec_push_en(vec, 2, i));
    test_uint_eq(vec_size(vec), 2);
    test_int_list((int*)vec_first(vec), i, 2);
}

TEST_CASE_ABORT(vec_push_args_invalid_magic)
{
    vec_push_args((vec_ct)&not_a_vector, 0);
}

TEST_CASE_FIXTURE(vec_push_args_nothing, vec_int1, vec)
{
    test_ptr_error(vec_push_args(vec, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_push_args, vec_int, vec)
{
    test_ptr_success(vec_push_args(vec, 2, &i[0], &i[1]));
    test_uint_eq(vec_size(vec), 2);
    test_int_list((int*)vec_first(vec), i, 2);
}

TEST_CASE_ABORT(vec_push_args_p_invalid_magic)
{
    vec_push_args_p((vec_ct)&not_a_vector, 0);
}

TEST_CASE_ABORT_FIXTURE(vec_push_args_p_invalid_type, vec_int, vec)
{
    vec_push_args_p(vec, 0);
}

TEST_CASE_FIXTURE(vec_push_args_p_nothing, vec_ptr1, vec)
{
    test_ptr_error(vec_push_args_p(vec, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_push_args_p, vec_ptr, vec)
{
    test_ptr_success(vec_push_args_p(vec, 2, pi[0], pi[1]));
    test_uint_eq(vec_size(vec), 2);
    test_ptr_list((int**)vec_first(vec), pi, 2);
}

TEST_CASE_ABORT(vec_insert_invalid_magic)
{
    vec_insert((vec_ct)&not_a_vector, 0);
}

TEST_CASE_FIXTURE(vec_insert_oob_positive_index, vec_int, vec)
{
    test_ptr_error(vec_insert(vec, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_oob_negative_index, vec_int, vec)
{
    test_ptr_error(vec_insert(vec, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_front_positive_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert(vec, 0));
    test_int_eq(vec_pos(vec, k), 0);
    test_int_eq(*(int*)vec_at(vec, 1), i[0]);
}

TEST_CASE_FIXTURE(vec_insert_back_positive_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert(vec, 10));
    test_int_eq(vec_pos(vec, k), 10);
}

TEST_CASE_FIXTURE(vec_insert_front_negative_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert(vec, -10));
    test_int_eq(vec_pos(vec, k), 0);
    test_int_eq(*(int*)vec_at(vec, 1), i[0]);
}

TEST_CASE_FIXTURE(vec_insert_back_negative_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert(vec, -1));
    test_int_eq(vec_pos(vec, k), 9); // -1 inserts before last elem
    test_int_eq(*(int*)vec_at(vec, 10), i[9]);
}

TEST_CASE_ABORT(vec_insert_e_invalid_magic)
{
    vec_insert_e((vec_ct)&not_a_vector, 0, NULL);
}

TEST_CASE_FIXTURE(vec_insert_e_oob_positive_index, vec_int, vec)
{
    test_ptr_error(vec_insert_e(vec, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_e_oob_negative_index, vec_int, vec)
{
    test_ptr_error(vec_insert_e(vec, -1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_e_front_positive_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_e(vec, 0, &i[5]));
    test_int_eq(vec_pos(vec, k), 0);
    test_int_eq(*k, i[5]);
    test_int_eq(*(int*)vec_at(vec, 1), i[0]);
}

TEST_CASE_FIXTURE(vec_insert_e_back_positive_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_e(vec, 10, &i[5]));
    test_int_eq(vec_pos(vec, k), 10);
    test_int_eq(*k, i[5]);
}

TEST_CASE_FIXTURE(vec_insert_e_front_negative_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_e(vec, -10, &i[5]));
    test_int_eq(vec_pos(vec, k), 0);
    test_int_eq(*k, i[5]);
    test_int_eq(*(int*)vec_at(vec, 1), i[0]);
}

TEST_CASE_FIXTURE(vec_insert_e_back_negative_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_e(vec, -1, &i[5]));
    test_int_eq(vec_pos(vec, k), 9); // -1 inserts before last elem
    test_int_eq(*k, i[5]);
    test_int_eq(*(int*)vec_at(vec, 10), i[9]);
}

TEST_CASE_ABORT(vec_insert_p_invalid_magic)
{
    vec_insert_p((vec_ct)&not_a_vector, 0, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_p_invalid_type, vec_int, vec)
{
    vec_insert_p(vec, 0, NULL);
}

TEST_CASE_FIXTURE(vec_insert_p_oob_positive_index, vec_ptr, vec)
{
    test_ptr_error(vec_insert_p(vec, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_p_oob_negative_index, vec_ptr, vec)
{
    test_ptr_error(vec_insert_p(vec, -1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_p_front_positive_index, vec_ptr10, vec)
{
    test_ptr_success(pk = vec_insert_p(vec, 0, pi[5]));
    test_int_eq(vec_pos(vec, pk), 0);
    test_ptr_eq(*pk, pi[5]);
    test_ptr_eq(vec_at_p(vec, 1), pi[0]);
}

TEST_CASE_FIXTURE(vec_insert_p_back_positive_index, vec_ptr10, vec)
{
    test_ptr_success(pk = vec_insert_p(vec, 10, pi[5]));
    test_int_eq(vec_pos(vec, pk), 10);
    test_ptr_eq(*pk, pi[5]);
}

TEST_CASE_FIXTURE(vec_insert_p_front_negative_index, vec_ptr10, vec)
{
    test_ptr_success(pk = vec_insert_p(vec, -10, pi[5]));
    test_int_eq(vec_pos(vec, pk), 0);
    test_ptr_eq(*pk, pi[5]);
    test_ptr_eq(vec_at_p(vec, 1), pi[0]);
}

TEST_CASE_FIXTURE(vec_insert_p_back_negative_index, vec_ptr10, vec)
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

TEST_CASE_FIXTURE(vec_insert_n_oob_positive_index, vec_int, vec)
{
    test_ptr_error(vec_insert_n(vec, 1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_n_nothing_positive_index, vec_int1, vec)
{
    test_ptr_error(vec_insert_n(vec, 0, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_insert_n_nothing_negative_index, vec_int1, vec)
{
    test_ptr_error(vec_insert_n(vec, -1, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_insert_n_oob_negative_index, vec_int, vec)
{
    test_ptr_error(vec_insert_n(vec, -1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_n_front_positive_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_n(vec, 0, 2));
    test_int_eq(vec_pos(vec, k), 0);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int*)vec_at(vec, 2), i[0]);
}

TEST_CASE_FIXTURE(vec_insert_n_back_positive_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_n(vec, 10, 2));
    test_int_eq(vec_pos(vec, k), 10);
    test_uint_eq(vec_size(vec), 12);
}

TEST_CASE_FIXTURE(vec_insert_n_front_negative_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_n(vec, -10, 2));
    test_int_eq(vec_pos(vec, k), 0);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int*)vec_at(vec, 2), i[0]);
}

TEST_CASE_FIXTURE(vec_insert_n_back_negative_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_n(vec, -1, 2));
    test_int_eq(vec_pos(vec, k), 9); // -1 inserts before last elem
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int*)vec_at(vec, 11), i[9]);
}

TEST_CASE_ABORT(vec_insert_en_invalid_magic)
{
    vec_insert_en((vec_ct)&not_a_vector, 0, 1, NULL);
}

TEST_CASE_FIXTURE(vec_insert_en_oob_positive_index, vec_int, vec)
{
    test_ptr_error(vec_insert_en(vec, 1, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_en_oob_negative_index, vec_int, vec)
{
    test_ptr_error(vec_insert_en(vec, -1, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_en_nothing_positive_index, vec_int1, vec)
{
    test_ptr_error(vec_insert_en(vec, 0, 0, NULL), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_insert_en_nothing_negative_index, vec_int1, vec)
{
    test_ptr_error(vec_insert_en(vec, -1, 0, NULL), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_insert_en_front_positive_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_en(vec, 0, 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 0);
    test_uint_eq(vec_size(vec), 12);
    test_int_list(k, &i[5], 2);
    test_int_eq(*(int*)vec_at(vec, 2), i[0]);
}

TEST_CASE_FIXTURE(vec_insert_en_back_positive_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_en(vec, 10, 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 10);
    test_uint_eq(vec_size(vec), 12);
    test_int_list(k, &i[5], 2);
}

TEST_CASE_FIXTURE(vec_insert_en_front_negative_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_en(vec, -10, 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 0);
    test_uint_eq(vec_size(vec), 12);
    test_int_list(k, &i[5], 2);
    test_int_eq(*(int*)vec_at(vec, 2), i[0]);
}

TEST_CASE_FIXTURE(vec_insert_en_back_negative_index, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_en(vec, -1, 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 9); // -1 inserts before last elem
    test_uint_eq(vec_size(vec), 12);
    test_int_list(k, &i[5], 2);
    test_int_eq(*(int*)vec_at(vec, 11), i[9]);
}

TEST_CASE_ABORT(vec_insert_before_invalid_magic)
{
    vec_insert_before((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_before_invalid_elem, vec_int, vec)
{
    vec_insert_before(vec, NULL);
}

TEST_CASE_FIXTURE(vec_insert_before_no_member, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_before(vec, k-1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_before_member_invalid_alignment, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_before(vec, ((char*)k)+1), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_insert_before, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_before(vec, vec_at(vec, 1)));
    test_int_eq(vec_pos(vec, k), 1);
    test_uint_eq(vec_size(vec), 11);
    test_int_eq(*(int*)vec_at(vec, 2), i[1]);
}

TEST_CASE_ABORT(vec_insert_before_e_invalid_magic)
{
    vec_insert_before_e((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_before_e_invalid_elem, vec_int, vec)
{
    vec_insert_before_e(vec, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_insert_before_e_no_member, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_before_e(vec, k-1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_before_e_member_invalid_alignment, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_before_e(vec, ((char*)k)+1, NULL), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_insert_before_e, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_before_e(vec, vec_at(vec, 1), &i[5]));
    test_int_eq(vec_pos(vec, k), 1);
    test_int_eq(*k, i[5]);
    test_uint_eq(vec_size(vec), 11);
    test_int_eq(*(int*)vec_at(vec, 2), i[1]);
}

TEST_CASE_ABORT(vec_insert_before_p_invalid_magic)
{
    vec_insert_before_p((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_before_p_invalid_elem, vec_ptr, vec)
{
    vec_insert_before_p(vec, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_before_p_invalid_type, vec_int, vec)
{
    vec_insert_before_p(vec, vec_at(vec, 0), NULL);
}

TEST_CASE_FIXTURE(vec_insert_before_p_no_member, vec_ptr10, vec)
{
    pk = vec_at(vec, 0);
    test_ptr_error(vec_insert_before_p(vec, pk-1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_before_p_member_invalid_alignment, vec_ptr10, vec)
{
    pk = vec_at(vec, 0);
    test_ptr_error(vec_insert_before_p(vec, ((char*)pk)+1, NULL), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_insert_before_p, vec_ptr10, vec)
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

TEST_CASE_ABORT_FIXTURE(vec_insert_before_n_invalid_elem, vec_int, vec)
{
    vec_insert_before_n(vec, NULL, 1);
}

TEST_CASE_FIXTURE(vec_insert_before_n_no_member, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_before_n(vec, k-1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_before_n_member_invalid_alignment, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_before_n(vec, ((char*)k)+1, 1), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_insert_before_n_nothing, vec_int1, vec)
{
    test_ptr_error(vec_insert_before_n(vec, vec_first(vec), 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_insert_before_n, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_before_n(vec, vec_at(vec, 1), 2));
    test_int_eq(vec_pos(vec, k), 1);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int*)vec_at(vec, 3), i[1]);
}

TEST_CASE_ABORT(vec_insert_before_en_invalid_magic)
{
    vec_insert_before_en((vec_ct)&not_a_vector, NULL, 1, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_before_en_invalid_elem, vec_int, vec)
{
    vec_insert_before_en(vec, NULL, 1, NULL);
}

TEST_CASE_FIXTURE(vec_insert_before_en_no_member, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_before_en(vec, k-1, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_before_en_member_invalid_alignment, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_before_en(vec, ((char*)k)+1, 1, NULL), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_insert_before_en_nothing, vec_int1, vec)
{
    test_ptr_error(vec_insert_before_en(vec, vec_first(vec), 0, NULL), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_insert_before_en, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_before_en(vec, vec_at(vec, 1), 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 1);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int*)vec_at(vec, 3), i[1]);
    test_int_list(k, &i[5], 2);
}

TEST_CASE_ABORT(vec_insert_after_invalid_magic)
{
    vec_insert_after((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_after_invalid_elem, vec_int, vec)
{
    vec_insert_after(vec, NULL);
}

TEST_CASE_FIXTURE(vec_insert_after_no_member, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_after(vec, k-1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_after_member_invalid_alignment, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_after(vec, ((char*)k)+1), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_insert_after, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_after(vec, vec_at(vec, 1)));
    test_int_eq(vec_pos(vec, k), 2);
    test_uint_eq(vec_size(vec), 11);
    test_int_eq(*(int*)vec_at(vec, 3), i[2]);
}

TEST_CASE_ABORT(vec_insert_after_e_invalid_magic)
{
    vec_insert_after_e((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_after_e_invalid_elem, vec_int, vec)
{
    vec_insert_after_e(vec, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_insert_after_e_no_member, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_after_e(vec, k-1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_after_e_member_invalid_alignment, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_after_e(vec, ((char*)k)+1, NULL), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_insert_after_e, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_after_e(vec, vec_at(vec, 1), &i[5]));
    test_int_eq(vec_pos(vec, k), 2);
    test_int_eq(*k, i[5]);
    test_uint_eq(vec_size(vec), 11);
    test_int_eq(*(int*)vec_at(vec, 3), i[2]);
}

TEST_CASE_ABORT(vec_insert_after_p_invalid_magic)
{
    vec_insert_after_p((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_after_p_invalid_elem, vec_ptr, vec)
{
    vec_insert_after_p(vec, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_after_p_invalid_type, vec_int, vec)
{
    vec_insert_after_p(vec, vec_at(vec, 0), NULL);
}

TEST_CASE_FIXTURE(vec_insert_after_p_no_member, vec_ptr10, vec)
{
    pk = vec_at(vec, 0);
    test_ptr_error(vec_insert_after_p(vec, pk-1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_after_p_member_invalid_alignment, vec_ptr10, vec)
{
    pk = vec_at(vec, 0);
    test_ptr_error(vec_insert_after_p(vec, ((char*)pk)+1, NULL), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_insert_after_p, vec_ptr10, vec)
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

TEST_CASE_ABORT_FIXTURE(vec_insert_after_n_invalid_elem, vec_int, vec)
{
    vec_insert_after_n(vec, NULL, 1);
}

TEST_CASE_FIXTURE(vec_insert_after_n_no_member, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_after_n(vec, k-1, 1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_after_n_member_invalid_alignment, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_after_n(vec, ((char*)k)+1, 1), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_insert_after_n_nothing, vec_int1, vec)
{
    test_ptr_error(vec_insert_after_n(vec, vec_first(vec), 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_insert_after_n, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_after_n(vec, vec_at(vec, 1), 2));
    test_int_eq(vec_pos(vec, k), 2);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int*)vec_at(vec, 4), i[2]);
}

TEST_CASE_ABORT(vec_insert_after_en_invalid_magic)
{
    vec_insert_after_en((vec_ct)&not_a_vector, NULL, 1, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_insert_after_en_invalid_elem, vec_int, vec)
{
    vec_insert_after_en(vec, NULL, 1, NULL);
}

TEST_CASE_FIXTURE(vec_insert_after_en_no_member, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_after_en(vec, k-1, 1, NULL), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_insert_after_en_member_invalid_alignment, vec_int10, vec)
{
    k = vec_at(vec, 0);
    test_ptr_error(vec_insert_after_en(vec, ((char*)k)+1, 1, NULL), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_insert_after_en_nothing, vec_int1, vec)
{
    test_ptr_error(vec_insert_after_en(vec, vec_first(vec), 0, NULL), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_insert_after_en, vec_int10, vec)
{
    test_ptr_success(k = vec_insert_after_en(vec, vec_at(vec, 1), 2, &i[5]));
    test_int_eq(vec_pos(vec, k), 2);
    test_uint_eq(vec_size(vec), 12);
    test_int_eq(*(int*)vec_at(vec, 4), i[2]);
    test_int_list(k, &i[5], 2);
}

TEST_CASE_ABORT(vec_pop_invalid_magic)
{
    vec_pop((vec_ct)&not_a_vector);
}

TEST_CASE_FIXTURE(vec_pop_empty_vector, vec_int, vec)
{
    test_int_error(vec_pop(vec), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_pop, vec_int10, vec)
{
    test_int_success(vec_pop(vec));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
}

TEST_CASE_ABORT(vec_pop_e_invalid_magic)
{
    vec_pop_e((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_FIXTURE(vec_pop_e_empty_vector, vec_int, vec)
{
    test_int_error(vec_pop_e(vec, NULL), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_pop_e, vec_int10, vec)
{
    test_int_success(vec_pop_e(vec, j));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
    test_int_eq(j[0], i[9]);
}

TEST_CASE_ABORT(vec_pop_p_invalid_magic)
{
    vec_pop_p((vec_ct)&not_a_vector);
}

TEST_CASE_ABORT_FIXTURE(vec_pop_p_invalid_type, vec_int, vec)
{
    vec_pop_p(vec);
}

TEST_CASE_FIXTURE(vec_pop_p_empty_vector, vec_ptr, vec)
{
    test_ptr_error(vec_pop_p(vec), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_pop_p, vec_ptr10, vec)
{
    test_ptr_success(k = vec_pop_p(vec));
    test_uint_eq(vec_size(vec), 9);
    test_ptr_eq(*(int**)vec_last(vec), pi[8]);
    test_ptr_eq(k, pi[9]);
}

static void _test_vec_dtor(vec_const_ct v, void *elem, void *ctx)
{
    int *count = ctx;
    
    count[0]++;
}

TEST_CASE_ABORT(vec_pop_f_invalid_magic)
{
    vec_pop_f((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_pop_f_empty_vector, vec_int, vec)
{
    test_int_error(vec_pop_f(vec, _test_vec_dtor, &count), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_pop_f, vec_int10, vec)
{
    test_int_success(vec_pop_f(vec, _test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(vec_pop_n_invalid_magic)
{
    vec_pop_n((vec_ct)&not_a_vector, 2);
}

TEST_CASE_FIXTURE(vec_pop_n_empty_vector, vec_int, vec)
{
    test_int_error(vec_pop_n(vec, 2), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_pop_n_nothing, vec_int1, vec)
{
    test_int_error(vec_pop_n(vec, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_pop_n, vec_int10, vec)
{
    test_rc_success(vec_pop_n(vec, 2), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_last(vec), i[7]);
}

TEST_CASE_FIXTURE(vec_pop_n_fewer_members, vec_int1, vec)
{
    test_rc_success(vec_pop_n(vec, 2), 1);
    test_uint_eq(vec_size(vec), 0);
}

TEST_CASE_ABORT(vec_pop_en_invalid_magic)
{
    vec_pop_en((vec_ct)&not_a_vector, NULL, 2);
}

TEST_CASE_FIXTURE(vec_pop_en_empty_vector, vec_int, vec)
{
    test_int_error(vec_pop_en(vec, NULL, 2), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_pop_en_nothing, vec_int1, vec)
{
    test_int_error(vec_pop_en(vec, NULL, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_pop_en, vec_int10, vec)
{
    test_rc_success(vec_pop_en(vec, j, 2), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_last(vec), i[7]);
    test_int_list(j, &i[8], 2);
}

TEST_CASE_FIXTURE(vec_pop_en_fewer_members, vec_int1, vec)
{
    test_rc_success(vec_pop_en(vec, j, 2), 1);
    test_uint_eq(vec_size(vec), 0);
    test_int_eq(j[0], i[0]);
}

TEST_CASE_ABORT(vec_pop_fn_invalid_magic)
{
    vec_pop_fn((vec_ct)&not_a_vector, 2, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_pop_fn_empty_vector, vec_int, vec)
{
    test_int_error(vec_pop_fn(vec, 2, _test_vec_dtor, &count), E_VEC_EMPTY);
}

TEST_CASE_FIXTURE(vec_pop_fn_nothing, vec_int1, vec)
{
    test_int_error(vec_pop_fn(vec, 0, _test_vec_dtor, &count), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_pop_fn, vec_int10, vec)
{
    test_rc_success(vec_pop_fn(vec, 2, _test_vec_dtor, &count), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_last(vec), i[7]);
    test_int_eq(count, 2);
}

TEST_CASE_FIXTURE(vec_pop_fn_fewer_members, vec_int1, vec)
{
    test_rc_success(vec_pop_fn(vec, 2, _test_vec_dtor, &count), 1);
    test_uint_eq(vec_size(vec), 0);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(vec_remove_invalid_magic)
{
    vec_remove((vec_ct)&not_a_vector, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_remove_invalid_elem, vec_int, vec)
{
    vec_remove(vec, NULL);
}

TEST_CASE_FIXTURE(vec_remove_no_member, vec_int10, vec)
{
    k = vec_first(vec);
    test_int_error(vec_remove(vec, k-1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_member_invalid_alignment, vec_int10, vec)
{
    k = vec_first(vec);
    test_int_error(vec_remove(vec, ((char*)k)+1), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_remove, vec_int10, vec)
{
    test_int_success(vec_remove(vec, vec_first(vec)));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_first(vec), i[1]);
}

TEST_CASE_ABORT(vec_remove_n_invalid_magic)
{
    vec_remove_n((vec_ct)&not_a_vector, NULL, 2);
}

TEST_CASE_ABORT_FIXTURE(vec_remove_n_invalid_elem, vec_int, vec)
{
    vec_remove_n(vec, NULL, 2);
}

TEST_CASE_FIXTURE(vec_remove_n_no_member, vec_int10, vec)
{
    k = vec_first(vec);
    test_int_error(vec_remove_n(vec, k-1, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_n_member_invalid_alignment, vec_int10, vec)
{
    k = vec_first(vec);
    test_int_error(vec_remove_n(vec, ((char*)k)+1, 2), E_VEC_UNALIGNED);
}

TEST_CASE_FIXTURE(vec_remove_n_nothing, vec_int1, vec)
{
    test_int_error(vec_remove_n(vec, vec_first(vec), 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_remove_n, vec_int10, vec)
{
    test_rc_success(vec_remove_n(vec, vec_first(vec), 2), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_first(vec), i[2]);
}

TEST_CASE_FIXTURE(vec_remove_n_fewer_members, vec_int1, vec)
{
    test_rc_success(vec_remove_n(vec, vec_first(vec), 2), 1);
    test_uint_eq(vec_size(vec), 0);
}

TEST_CASE_ABORT(vec_remove_at_invalid_magic)
{
    vec_remove_at((vec_ct)&not_a_vector, 0);
}

TEST_CASE_FIXTURE(vec_remove_at_oob_positive_index, vec_int, vec)
{
    test_int_error(vec_remove_at(vec, 0), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_oob_negative_index, vec_int, vec)
{
    test_int_error(vec_remove_at(vec, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_positive_index, vec_int10, vec)
{
    test_int_success(vec_remove_at(vec, 0));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_first(vec), i[1]);
}

TEST_CASE_FIXTURE(vec_remove_at_negative_index, vec_int10, vec)
{
    test_int_success(vec_remove_at(vec, -1));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
}

TEST_CASE_ABORT(vec_remove_at_e_invalid_magic)
{
    vec_remove_at_e((vec_ct)&not_a_vector, NULL, 0);
}

TEST_CASE_FIXTURE(vec_remove_at_e_oob_positive_index, vec_int, vec)
{
    test_int_error(vec_remove_at_e(vec, NULL, 0), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_e_oob_negative_index, vec_int, vec)
{
    test_int_error(vec_remove_at_e(vec, NULL, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_e_positive_index, vec_int10, vec)
{
    test_int_success(vec_remove_at_e(vec, j, 0));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_first(vec), i[1]);
    test_int_eq(j[0], i[0]);
}

TEST_CASE_FIXTURE(vec_remove_at_e_negative_index, vec_int10, vec)
{
    test_int_success(vec_remove_at_e(vec, j, -1));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
    test_int_eq(j[0], i[9]);
}

TEST_CASE_ABORT(vec_remove_at_p_invalid_magic)
{
    vec_remove_at_p((vec_ct)&not_a_vector, 0);
}

TEST_CASE_ABORT_FIXTURE(vec_remove_at_p_invalid_type, vec_int, vec)
{
    vec_remove_at_p(vec, 0);
}

TEST_CASE_FIXTURE(vec_remove_at_p_oob_positive_index, vec_ptr, vec)
{
    test_ptr_error(vec_remove_at_p(vec, 0), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_p_oob_negative_index, vec_ptr, vec)
{
    test_ptr_error(vec_remove_at_p(vec, -1), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_p_positive_index, vec_ptr10, vec)
{
    test_ptr_success(k = vec_remove_at_p(vec, 0));
    test_uint_eq(vec_size(vec), 9);
    test_ptr_eq(vec_first_p(vec), pi[1]);
    test_ptr_eq(k, pi[0]);
}

TEST_CASE_FIXTURE(vec_remove_at_p_negative_index, vec_ptr10, vec)
{
    test_ptr_success(k = vec_remove_at_p(vec, -1));
    test_uint_eq(vec_size(vec), 9);
    test_ptr_eq(vec_last_p(vec), pi[8]);
    test_ptr_eq(k, pi[9]);
}

TEST_CASE_ABORT(vec_remove_at_f_invalid_magic)
{
    vec_remove_at_f((vec_ct)&not_a_vector, 0, _test_vec_dtor, &count);
}

TEST_CASE_FIXTURE(vec_remove_at_f_oob_positive_index, vec_int, vec)
{
    test_int_error(vec_remove_at_f(vec, 0, _test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_f_oob_negative_index, vec_int, vec)
{
    test_int_error(vec_remove_at_f(vec, -1, _test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_f_positive_index, vec_int10, vec)
{
    test_int_success(vec_remove_at_f(vec, 0, _test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_first(vec), i[1]);
    test_int_eq(count, 1);
}

TEST_CASE_FIXTURE(vec_remove_at_f_negative_index, vec_int10, vec)
{
    test_int_success(vec_remove_at_f(vec, -1, _test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(vec_remove_at_n_invalid_magic)
{
    vec_remove_at_n((vec_ct)&not_a_vector, 0, 2);
}

TEST_CASE_FIXTURE(vec_remove_at_n_oob_positive_index, vec_int, vec)
{
    test_int_error(vec_remove_at_n(vec, 0, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_n_oob_negative_index, vec_int, vec)
{
    test_int_error(vec_remove_at_n(vec, -1, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_n_nothing_positive_index, vec_int1, vec)
{
    test_int_error(vec_remove_at_n(vec, 0, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_remove_at_n_nothing_negative_index, vec_int1, vec)
{
    test_int_error(vec_remove_at_n(vec, -1, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_remove_at_n_positive_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_n(vec, 0, 2), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_first(vec), i[2]);
}

TEST_CASE_FIXTURE(vec_remove_at_n_fewer_members_positive_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_n(vec, 9, 2), 1);
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
}

TEST_CASE_FIXTURE(vec_remove_at_n_negative_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_n(vec, -2, 2), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_last(vec), i[7]);
}

TEST_CASE_FIXTURE(vec_remove_at_n_fewer_members_negative_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_n(vec, -1, 2), 1);
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
}

TEST_CASE_ABORT(vec_remove_at_en_invalid_magic)
{
    vec_remove_at_en((vec_ct)&not_a_vector, j, 0, 2);
}

TEST_CASE_FIXTURE(vec_remove_at_en_oob_positive_index, vec_int, vec)
{
    test_int_error(vec_remove_at_en(vec, j, 0, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_en_oob_negative_index, vec_int, vec)
{
    test_int_error(vec_remove_at_en(vec, j, -1, 2), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_en_nothing_positive_index, vec_int1, vec)
{
    test_int_error(vec_remove_at_en(vec, j, 0, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_remove_at_en_nothing_negative_index, vec_int1, vec)
{
    test_int_error(vec_remove_at_en(vec, j, -1, 0), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_remove_at_en_positive_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_en(vec, j, 0, 2), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_first(vec), i[2]);
    test_int_list(j, i, 2);
}

TEST_CASE_FIXTURE(vec_remove_at_en_fewer_members_positive_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_en(vec, j, 9, 2), 1);
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
    test_int_eq(j[0], i[9]);
}

TEST_CASE_FIXTURE(vec_remove_at_en_negative_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_en(vec, j, -2, 2), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_last(vec), i[7]);
    test_int_list(j, &i[8], 2);
}

TEST_CASE_FIXTURE(vec_remove_at_en_fewer_members_negative_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_en(vec, j, -1, 2), 1);
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
    test_int_eq(j[0], i[9]);
}

TEST_CASE_ABORT(vec_remove_at_fn_invalid_magic)
{
    vec_remove_at_fn((vec_ct)&not_a_vector, 0, 2, _test_vec_dtor, &count);
}

TEST_CASE_FIXTURE(vec_remove_at_fn_oob_positive_index, vec_int, vec)
{
    test_int_error(vec_remove_at_fn(vec, 0, 2, _test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_fn_oob_negative_index, vec_int, vec)
{
    test_int_error(vec_remove_at_fn(vec, -1, 2, _test_vec_dtor, &count), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_remove_at_fn_nothing_positive_index, vec_int1, vec)
{
    test_int_error(vec_remove_at_fn(vec, 0, 0, _test_vec_dtor, &count), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_remove_at_fn_nothing_negative_index, vec_int1, vec)
{
    test_int_error(vec_remove_at_fn(vec, -1, 0, _test_vec_dtor, &count), E_VEC_NULL_REQUEST);
}

TEST_CASE_FIXTURE(vec_remove_at_fn_positive_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_fn(vec, 0, 2, _test_vec_dtor, &count), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_first(vec), i[2]);
    test_int_eq(count, 2);
}

TEST_CASE_FIXTURE(vec_remove_at_fn_fewer_members_positive_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_fn(vec, 9, 2, _test_vec_dtor, &count), 1);
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
    test_int_eq(count, 1);
}

TEST_CASE_FIXTURE(vec_remove_at_fn_negative_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_fn(vec, -2, 2, _test_vec_dtor, &count), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_last(vec), i[7]);
    test_int_eq(count, 2);
}

TEST_CASE_FIXTURE(vec_remove_at_fn_fewer_members_negative_index, vec_int10, vec)
{
    test_rc_success(vec_remove_at_fn(vec, -1, 2, _test_vec_dtor, &count), 1);
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_last(vec), i[8]);
    test_int_eq(count, 1);
}

static bool _test_vec_pred_int(vec_const_ct v, const void *elem, void *ctx)
{
    const int *i1 = elem, *i2 = ctx;
    
    return *i1 == *i2;
}

static bool _test_vec_pred_ptr(vec_const_ct v, const void *elem, void *ctx)
{
    const int *const *p1 = elem, *const *p2 = ctx;
    
    return *p1 == *p2;
}

TEST_CASE_ABORT(vec_find_invalid_magic)
{
    vec_find((vec_ct)&not_a_vector, _test_vec_pred_int, (void*)&i[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_invalid_pred, vec_int, vec)
{
    vec_find((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_fail, vec_int, vec)
{
    test_ptr_error(vec_find(vec, _test_vec_pred_int, (void*)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find, vec_int10, vec)
{
    test_ptr_success(k = vec_find(vec, _test_vec_pred_int, (void*)&i[2]));
    test_int_eq(vec_pos(vec, k), 2);
}

TEST_CASE_ABORT(vec_find_p_invalid_magic)
{
    vec_find_p((vec_ct)&not_a_vector, _test_vec_pred_ptr, (void*)&pi[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_p_invalid_pred, vec_ptr, vec)
{
    vec_find_p((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_find_p_invalid_type, vec_int, vec)
{
    vec_find_p(vec, _test_vec_pred_ptr, (void*)&pi[2]);
}

TEST_CASE_FIXTURE(vec_find_p_fail, vec_ptr, vec)
{
    test_ptr_error(vec_find_p(vec, _test_vec_pred_ptr, (void*)&pi[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_p, vec_ptr10, vec)
{
    test_ptr_success(pk = vec_find_p(vec, _test_vec_pred_ptr, (void*)&pi[2]));
    test_ptr_eq(pk, pi[2]);
}

TEST_CASE_ABORT(vec_find_r_invalid_magic)
{
    vec_find_r((vec_ct)&not_a_vector, _test_vec_pred_int, (void*)&i[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_r_invalid_pred, vec_int, vec)
{
    vec_find_r((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_r_fail, vec_int, vec)
{
    test_ptr_error(vec_find_r(vec, _test_vec_pred_int, (void*)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_r, vec_int10, vec)
{
    test_ptr_success(k = vec_find_r(vec, _test_vec_pred_int, (void*)&i[2]));
    test_int_eq(vec_pos(vec, k), 7);
}

TEST_CASE_ABORT(vec_find_rp_invalid_magic)
{
    vec_find_rp((vec_ct)&not_a_vector, _test_vec_pred_ptr, (void*)&pi[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_rp_invalid_pred, vec_ptr, vec)
{
    vec_find_rp((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_find_rp_invalid_type, vec_int, vec)
{
    vec_find_rp(vec, _test_vec_pred_ptr, (void*)&pi[2]);
}

TEST_CASE_FIXTURE(vec_find_rp_fail, vec_ptr, vec)
{
    test_ptr_error(vec_find_rp(vec, _test_vec_pred_ptr, (void*)&pi[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_rp, vec_ptr10, vec)
{
    test_ptr_success(pk = vec_find_rp(vec, _test_vec_pred_ptr, (void*)&pi[2]));
    test_ptr_eq(pk, pi[7]);
}

TEST_CASE_ABORT(vec_find_pos_invalid_magic)
{
    vec_find_pos((vec_ct)&not_a_vector, _test_vec_pred_int, (void*)&i[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_pos_invalid_pred, vec_int, vec)
{
    vec_find_pos(vec, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_pos_fail, vec_int, vec)
{
    test_int_error(vec_find_pos(vec, _test_vec_pred_int, (void*)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_pos, vec_int10, vec)
{
    test_rc_success(vec_find_pos(vec, _test_vec_pred_int, (void*)&i[2]), 2);
}

TEST_CASE_ABORT(vec_find_pos_r_invalid_magic)
{
    vec_find_pos_r((vec_ct)&not_a_vector, _test_vec_pred_int, (void*)&i[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_pos_r_invalid_pred, vec_int, vec)
{
    vec_find_pos_r(vec, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_pos_r_fail, vec_int, vec)
{
    test_int_error(vec_find_pos_r(vec, _test_vec_pred_int, (void*)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_pos_r, vec_int10, vec)
{
    test_rc_success(vec_find_pos_r(vec, _test_vec_pred_int, (void*)&i[2]), 7);
}

TEST_CASE_ABORT(vec_find_get_invalid_magic)
{
    vec_find_get((vec_ct)&not_a_vector, j, _test_vec_pred_int, (void*)&i[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_get_invalid_pred, vec_int, vec)
{
    vec_find_get(vec, NULL, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_get_fail, vec_int, vec)
{
    test_int_error(vec_find_get(vec, j, _test_vec_pred_int, (void*)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_get, vec_int10, vec)
{
    test_int_success(vec_find_get(vec, j, _test_vec_pred_int, (void*)&i[2]));
    test_int_eq(j[0], i[2]);
}

TEST_CASE_ABORT(vec_find_get_r_invalid_magic)
{
    vec_find_get_r((vec_ct)&not_a_vector, j, _test_vec_pred_int, (void*)&i[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_get_r_invalid_pred, vec_int, vec)
{
    vec_find_get_r(vec, NULL, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_get_r_fail, vec_int, vec)
{
    test_int_error(vec_find_get_r(vec, j, _test_vec_pred_int, (void*)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_get_r, vec_int10, vec)
{
    test_int_success(vec_find_get_r(vec, j, _test_vec_pred_int, (void*)&i[2]));
    test_int_eq(j[0], i[7]);
}

TEST_CASE_ABORT(vec_find_remove_invalid_magic)
{
    vec_find_remove((vec_ct)&not_a_vector, _test_vec_pred_int, (void*)&i[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_remove_invalid_pred, vec_int, vec)
{
    vec_find_remove(vec, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_remove_fail, vec_int, vec)
{
    test_int_error(vec_find_remove(vec, _test_vec_pred_int, (void*)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_remove, vec_int10, vec)
{
    test_int_success(vec_find_remove(vec, _test_vec_pred_int, (void*)&i[2]));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_at(vec, 2), i[3]);
}

TEST_CASE_ABORT(vec_find_remove_p_invalid_magic)
{
    vec_find_remove_p((vec_ct)&not_a_vector, _test_vec_pred_ptr, (void*)&pi[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_remove_p_invalid_pred, vec_ptr, vec)
{
    vec_find_remove_p(vec, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_find_remove_p_invalid_type, vec_int, vec)
{
    vec_find_remove_p(vec, _test_vec_pred_ptr, (void*)&pi[2]);
}

TEST_CASE_FIXTURE(vec_find_remove_p_fail, vec_ptr, vec)
{
    test_ptr_error(vec_find_remove_p(vec, _test_vec_pred_ptr, (void*)&pi[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_remove_p, vec_ptr10, vec)
{
    test_ptr_success(pk = vec_find_remove_p(vec, _test_vec_pred_ptr, (void*)&pi[2]));
    test_uint_eq(vec_size(vec), 9);
    test_ptr_eq(pk, pi[2]);
    test_ptr_eq(vec_at_p(vec, 2), pi[3]);
}

TEST_CASE_ABORT(vec_find_remove_f_invalid_magic)
{
    vec_find_remove_f((vec_ct)&not_a_vector, _test_vec_pred_int, (void*)&i[2], _test_vec_dtor, &count);
}

TEST_CASE_ABORT_FIXTURE(vec_find_remove_f_invalid_pred, vec_int, vec)
{
    vec_find_remove_f(vec, NULL, NULL, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_remove_f_fail, vec_int, vec)
{
    test_int_error(vec_find_remove_f(vec, _test_vec_pred_int, (void*)&i[2], _test_vec_dtor, &count), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_remove_f, vec_int10, vec)
{
    test_int_success(vec_find_remove_f(vec, _test_vec_pred_int, (void*)&i[2], _test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_at(vec, 2), i[3]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(vec_find_remove_r_invalid_magic)
{
    vec_find_remove_r((vec_ct)&not_a_vector, _test_vec_pred_int, (void*)&i[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_remove_r_invalid_pred, vec_int, vec)
{
    vec_find_remove_r(vec, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_remove_r_fail, vec_int, vec)
{
    test_int_error(vec_find_remove_r(vec, _test_vec_pred_int, (void*)&i[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_remove_r, vec_int10, vec)
{
    test_int_success(vec_find_remove_r(vec, _test_vec_pred_int, (void*)&i[2]));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_at(vec, 7), i[8]);
}

TEST_CASE_ABORT(vec_find_remove_rp_invalid_magic)
{
    vec_find_remove_rp((vec_ct)&not_a_vector, _test_vec_pred_ptr, (void*)&pi[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_remove_rp_invalid_pred, vec_ptr, vec)
{
    vec_find_remove_rp(vec, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_find_remove_rp_invalid_type, vec_int, vec)
{
    vec_find_remove_rp(vec, _test_vec_pred_ptr, (void*)&pi[2]);
}

TEST_CASE_FIXTURE(vec_find_remove_rp_fail, vec_ptr, vec)
{
    test_ptr_error(vec_find_remove_rp(vec, _test_vec_pred_ptr, (void*)&pi[2]), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_remove_rp, vec_ptr10, vec)
{
    test_ptr_success(pk = vec_find_remove_rp(vec, _test_vec_pred_ptr, (void*)&pi[2]));
    test_uint_eq(vec_size(vec), 9);
    test_ptr_eq(pk, pi[7]);
    test_ptr_eq(vec_at_p(vec, 7), pi[8]);
}

TEST_CASE_ABORT(vec_find_remove_rf_invalid_magic)
{
    vec_find_remove_rf((vec_ct)&not_a_vector, _test_vec_pred_int, (void*)&i[2], _test_vec_dtor, &count);
}

TEST_CASE_ABORT_FIXTURE(vec_find_remove_rf_invalid_pred, vec_int, vec)
{
    vec_find_remove_rf(vec, NULL, NULL, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_remove_rf_fail, vec_int, vec)
{
    test_int_error(vec_find_remove_rf(vec, _test_vec_pred_int, (void*)&i[2], _test_vec_dtor, &count), E_VEC_NOT_FOUND);
}

TEST_CASE_FIXTURE(vec_find_remove_rf, vec_int10, vec)
{
    test_int_success(vec_find_remove_rf(vec, _test_vec_pred_int, (void*)&i[2], _test_vec_dtor, &count));
    test_uint_eq(vec_size(vec), 9);
    test_int_eq(*(int*)vec_at(vec, 7), i[8]);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(vec_find_remove_all_invalid_magic)
{
    vec_find_remove_all((vec_ct)&not_a_vector, _test_vec_pred_int, (void*)&i[2]);
}

TEST_CASE_ABORT_FIXTURE(vec_find_remove_all_invalid_pred, vec_int, vec)
{
    vec_find_remove_all(vec, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_remove_all, vec_int10, vec)
{
    test_rc_success(vec_find_remove_all(vec, _test_vec_pred_int, (void*)&i[2]), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_at(vec, 2), i[3]);
    test_int_eq(*(int*)vec_at(vec, 6), i[8]);
}

TEST_CASE_ABORT(vec_find_remove_all_f_invalid_magic)
{
    vec_find_remove_all_f((vec_ct)&not_a_vector, _test_vec_pred_int, (void*)&i[2], _test_vec_dtor, &count);
}

TEST_CASE_ABORT_FIXTURE(vec_find_remove_all_f_invalid_pred, vec_int, vec)
{
    vec_find_remove_all_f(vec, NULL, NULL, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_find_remove_all_f, vec_int10, vec)
{
    test_rc_success(vec_find_remove_all_f(vec, _test_vec_pred_int, (void*)&i[2], _test_vec_dtor, &count), 2);
    test_uint_eq(vec_size(vec), 8);
    test_int_eq(*(int*)vec_at(vec, 2), i[3]);
    test_int_eq(*(int*)vec_at(vec, 6), i[8]);
    test_int_eq(count, 2);
}

TEST_CASE_ABORT(vec_swap_invalid_magic)
{
    vec_swap((vec_ct)&not_a_vector, 0, 9);
}

TEST_CASE_FIXTURE(vec_swap_oob_positive_index, vec_int, vec)
{
    test_int_error(vec_swap(vec, 0, 9), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_swap_oob_negative_index, vec_int, vec)
{
    test_int_error(vec_swap(vec, -1, -10), E_VEC_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(vec_swap_positive_index, vec_int10, vec)
{
    test_int_success(vec_swap(vec, 0, 9));
    test_int_eq(*(int*)vec_first(vec), i[9]);
    test_int_eq(*(int*)vec_last(vec), i[0]);
}

TEST_CASE_FIXTURE(vec_swap_negative_index, vec_int10, vec)
{
    test_int_success(vec_swap(vec, -1, -10));
    test_int_eq(*(int*)vec_first(vec), i[9]);
    test_int_eq(*(int*)vec_last(vec), i[0]);
}

TEST_CASE_ABORT(vec_get_buffer_invalid_magic)
{
    vec_get_buffer((vec_ct)&not_a_vector, NULL, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_get_buffer_invalid_buffer, vec_int, vec)
{
    vec_get_buffer(vec, NULL, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_get_buffer_no_capacity, vec_int, vec)
{
    void *buf;
    test_int_error(vec_get_buffer(vec, &buf, NULL, NULL), E_VEC_NO_BUFFER);
}

TEST_CASE_FIXTURE(vec_get_buffer, vec_int10, vec)
{
    void *buf;
    size_t size, cap;
    
    test_int_success(vec_get_buffer(vec, &buf, &size, &cap));
    test_uint_eq(size, 10);
    test_uint_ge(cap, 10);
    test_uint_eq(vec_size(vec), 0);
    test_uint_eq(vec_capacity(vec), 0);
    test_int_list((int*)buf, i, 10);
    
    test_free(buf);
}

TEST_CASE_ABORT(vec_truncate_invalid_magic)
{
    vec_truncate((vec_ct)&not_a_vector, 1);
}

TEST_CASE_FIXTURE(vec_truncate, vec_int10, vec)
{
    test_uint_eq(vec_truncate(vec, 1), 9);
    test_uint_eq(vec_size(vec), 1);
    test_int_eq(*(int*)vec_first(vec), i[0]);
}

TEST_CASE_ABORT(vec_truncate_f_invalid_magic)
{
    vec_truncate_f((vec_ct)&not_a_vector, 1, _test_vec_dtor, &count);
}

TEST_CASE_FIXTURE(vec_truncate_f, vec_int10, vec)
{
    test_uint_eq(vec_truncate_f(vec, 1, _test_vec_dtor, &count), 9);
    test_uint_eq(vec_size(vec), 1);
    test_int_eq(*(int*)vec_first(vec), i[0]);
    test_int_eq(count, 9);
}

TEST_CASE_ABORT(vec_set_capacity_invalid_magic)
{
    vec_set_capacity((vec_ct)&not_a_vector, 1);
}

TEST_CASE_FIXTURE(vec_set_capacity_below_min_capacity, vec_int, vec)
{
    test_int_success(vec_set_capacity(vec, 1));
    test_uint_eq(vec_capacity(vec), vec_min_capacity(vec));
}

TEST_CASE_FIXTURE(vec_set_capacity, vec_int, vec)
{
    test_int_success(vec_set_capacity(vec, 2*vec_min_capacity(vec)));
    test_uint_eq(vec_capacity(vec), 2*vec_min_capacity(vec));
}

TEST_CASE_FIXTURE(vec_set_capacity_below_current_capacity, vec_int10, vec)
{
    test_int_success(vec_set_capacity(vec, 5));
    test_uint_eq(vec_capacity(vec), 5);
}

TEST_CASE_ABORT(vec_set_capacity_f_invalid_magic)
{
    vec_set_capacity_f((vec_ct)&not_a_vector, 1, _test_vec_dtor, &count);
}

TEST_CASE_FIXTURE(vec_set_capacity_f_below_min_capacity, vec_int, vec)
{
    test_int_success(vec_set_capacity_f(vec, 1, _test_vec_dtor, &count));
    test_uint_eq(vec_capacity(vec), vec_min_capacity(vec));
    test_int_eq(count, 0);
}

TEST_CASE_FIXTURE(vec_set_capacity_f, vec_int, vec)
{
    test_int_success(vec_set_capacity_f(vec, 2*vec_min_capacity(vec), _test_vec_dtor, &count));
    test_uint_eq(vec_capacity(vec), 2*vec_min_capacity(vec));
    test_int_eq(count, 0);
}

TEST_CASE_FIXTURE(vec_set_capacity_f_below_current_capacity, vec_int10, vec)
{
    test_int_success(vec_set_capacity_f(vec, 5, _test_vec_dtor, &count));
    test_uint_eq(vec_capacity(vec), 5);
    test_int_eq(count, 5);
}

static int _test_vec_fold(vec_const_ct v, size_t index, void *elem, void *ctx)
{
    int *i = elem, *sum = ctx;
    
    *sum = *sum*10 + *i;
    
    return 0;
}

TEST_CASE_ABORT(vec_fold_invalid_magic)
{
    vec_fold((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_fold_invalid_fold, vec_int, vec)
{
    vec_fold(vec, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_fold, vec_int, vec)
{
    int i[] = { 1, 2, 3 };
    
    vec_push_en(vec, 3, i);
    test_int_success(vec_fold(vec, _test_vec_fold, &count));
    test_int_eq(count, 123);
}

TEST_CASE_ABORT(vec_fold_r_invalid_magic)
{
    vec_fold_r((vec_ct)&not_a_vector, NULL, NULL);
}

TEST_CASE_ABORT_FIXTURE(vec_fold_r_invalid_fold, vec_int, vec)
{
    vec_fold_r(vec, NULL, NULL);
}

TEST_CASE_FIXTURE(vec_fold_r, vec_int, vec)
{
    int i[] = { 1, 2, 3 };
    
    vec_push_en(vec, 3, i);
    test_int_success(vec_fold_r(vec, _test_vec_fold, &count));
    test_int_eq(count, 321);
}


test_suite_ct test_suite_con_vec(void)
{
    return test_suite_new_with_cases("vec"
        , test_case_new(vec_new_invalid_capacity)
        
        , test_case_new(vec_elemsize_invalid_magic)
        , test_case_new(vec_elemsize)
        
        , test_case_new(vec_is_empty_invalid_magic)
        , test_case_new(vec_is_empty_on_init)
        , test_case_new(vec_is_not_empty_after_push)
        , test_case_new(vec_is_empty_after_push_pop)
        
        , test_case_new(vec_size_invalid_magic)
        , test_case_new(vec_size_zero_on_init)
        , test_case_new(vec_size_one_after_push)
        , test_case_new(vec_size_zero_after_push_pop)
        
        , test_case_new(vec_capacity_invalid_magic)
        , test_case_new(vec_capacity_zero_on_init)
        , test_case_new(vec_capacity_initial_capacity_after_push)
        , test_case_new(vec_capacity_double_capacity_after_push_above_capacity)
        , test_case_new(vec_capacity_initial_capacity_after_pop_below_threshold)
        
        , test_case_new(vec_pos_invalid_magic)
        , test_case_new(vec_pos_invalid_elem)
        , test_case_new(vec_pos_no_member)
        , test_case_new(vec_pos)
        , test_case_new(vec_pos_no_member_before)
        , test_case_new(vec_pos_no_member_after)
        , test_case_new(vec_pos_member_invalid_alignment)
        
        , test_case_new(vec_first_invalid_magic)
        , test_case_new(vec_first_empty_vector)
        , test_case_new(vec_first)
        , test_case_new(vec_first_p_invalid_magic)
        , test_case_new(vec_first_p_invalid_type)
        , test_case_new(vec_first_p_empty_vector)
        , test_case_new(vec_first_p)
        , test_case_new(vec_last_invalid_magic)
        , test_case_new(vec_last_empty_vector)
        , test_case_new(vec_last)
        , test_case_new(vec_last_p_invalid_magic)
        , test_case_new(vec_last_p_invalid_type)
        , test_case_new(vec_last_p_empty_vector)
        , test_case_new(vec_last_p)
        
        , test_case_new(vec_at_invalid_magic)
        , test_case_new(vec_at_empty_vector_positive_index)
        , test_case_new(vec_at_empty_vector_negative_index)
        , test_case_new(vec_at_positive_index)
        , test_case_new(vec_at_negative_index)
        , test_case_new(vec_at_p_invalid_magic)
        , test_case_new(vec_at_p_invalid_type)
        , test_case_new(vec_at_p_empty_vector_positive_index)
        , test_case_new(vec_at_p_empty_vector_negative_index)
        , test_case_new(vec_at_p_positive_index)
        , test_case_new(vec_at_p_negative_index)
        
        , test_case_new(vec_get_first_invalid_magic)
        , test_case_new(vec_get_first_empty_vector)
        , test_case_new(vec_get_first)
        , test_case_new(vec_get_last_invalid_magic)
        , test_case_new(vec_get_last_empty_vector)
        , test_case_new(vec_get_last)
        , test_case_new(vec_get_invalid_magic)
        , test_case_new(vec_get_empty_vector_positive_index)
        , test_case_new(vec_get_empty_vector_negative_index)
        , test_case_new(vec_get_positive_index)
        , test_case_new(vec_get_negative_index)
        , test_case_new(vec_get_n_invalid_magic)
        , test_case_new(vec_get_n_empty_vector_positive_index)
        , test_case_new(vec_get_n_empty_vector_negative_index)
        , test_case_new(vec_get_n_nothing_positive_index)
        , test_case_new(vec_get_n_nothing_negative_index)
        , test_case_new(vec_get_n_positive_index)
        , test_case_new(vec_get_n_negative_index)
        
        , test_case_new(vec_push_invalid_magic)
        , test_case_new(vec_push)
        , test_case_new(vec_push_e_invalid_magic)
        , test_case_new(vec_push_e)
        , test_case_new(vec_push_p_invalid_magic)
        , test_case_new(vec_push_p_invalid_type)
        , test_case_new(vec_push_p)
        , test_case_new(vec_push_n_invalid_magic)
        , test_case_new(vec_push_n_nothing)
        , test_case_new(vec_push_n)
        , test_case_new(vec_push_en_invalid_magic)
        , test_case_new(vec_push_en_nothing)
        , test_case_new(vec_push_en)
        , test_case_new(vec_push_args_invalid_magic)
        , test_case_new(vec_push_args_nothing)
        , test_case_new(vec_push_args)
        , test_case_new(vec_push_args_p_invalid_magic)
        , test_case_new(vec_push_args_p_invalid_type)
        , test_case_new(vec_push_args_p_nothing)
        , test_case_new(vec_push_args_p)
        
        , test_case_new(vec_insert_invalid_magic)
        , test_case_new(vec_insert_oob_positive_index)
        , test_case_new(vec_insert_oob_negative_index)
        , test_case_new(vec_insert_front_positive_index)
        , test_case_new(vec_insert_front_negative_index)
        , test_case_new(vec_insert_back_positive_index)
        , test_case_new(vec_insert_back_negative_index)
        
        , test_case_new(vec_insert_e_invalid_magic)
        , test_case_new(vec_insert_e_oob_positive_index)
        , test_case_new(vec_insert_e_oob_negative_index)
        , test_case_new(vec_insert_e_front_positive_index)
        , test_case_new(vec_insert_e_front_negative_index)
        , test_case_new(vec_insert_e_back_positive_index)
        , test_case_new(vec_insert_e_back_negative_index)
        
        , test_case_new(vec_insert_p_invalid_magic)
        , test_case_new(vec_insert_p_invalid_type)
        , test_case_new(vec_insert_p_oob_positive_index)
        , test_case_new(vec_insert_p_oob_negative_index)
        , test_case_new(vec_insert_p_front_positive_index)
        , test_case_new(vec_insert_p_front_negative_index)
        , test_case_new(vec_insert_p_back_positive_index)
        , test_case_new(vec_insert_p_back_negative_index)
        
        , test_case_new(vec_insert_n_invalid_magic)
        , test_case_new(vec_insert_n_oob_positive_index)
        , test_case_new(vec_insert_n_oob_negative_index)
        , test_case_new(vec_insert_n_nothing_positive_index)
        , test_case_new(vec_insert_n_nothing_negative_index)
        , test_case_new(vec_insert_n_front_positive_index)
        , test_case_new(vec_insert_n_front_negative_index)
        , test_case_new(vec_insert_n_back_positive_index)
        , test_case_new(vec_insert_n_back_negative_index)
        
        , test_case_new(vec_insert_en_invalid_magic)
        , test_case_new(vec_insert_en_oob_positive_index)
        , test_case_new(vec_insert_en_oob_negative_index)
        , test_case_new(vec_insert_en_nothing_positive_index)
        , test_case_new(vec_insert_en_nothing_negative_index)
        , test_case_new(vec_insert_en_front_positive_index)
        , test_case_new(vec_insert_en_front_negative_index)
        , test_case_new(vec_insert_en_back_positive_index)
        , test_case_new(vec_insert_en_back_negative_index)
        
        , test_case_new(vec_insert_before_invalid_magic)
        , test_case_new(vec_insert_before_invalid_elem)
        , test_case_new(vec_insert_before_no_member)
        , test_case_new(vec_insert_before_member_invalid_alignment)
        , test_case_new(vec_insert_before)
        , test_case_new(vec_insert_before_e_invalid_magic)
        , test_case_new(vec_insert_before_e_invalid_elem)
        , test_case_new(vec_insert_before_e_no_member)
        , test_case_new(vec_insert_before_e_member_invalid_alignment)
        , test_case_new(vec_insert_before_e)
        , test_case_new(vec_insert_before_p_invalid_magic)
        , test_case_new(vec_insert_before_p_invalid_elem)
        , test_case_new(vec_insert_before_p_invalid_type)
        , test_case_new(vec_insert_before_p_no_member)
        , test_case_new(vec_insert_before_p_member_invalid_alignment)
        , test_case_new(vec_insert_before_p)
        , test_case_new(vec_insert_before_n_invalid_magic)
        , test_case_new(vec_insert_before_n_invalid_elem)
        , test_case_new(vec_insert_before_n_no_member)
        , test_case_new(vec_insert_before_n_member_invalid_alignment)
        , test_case_new(vec_insert_before_n_nothing)
        , test_case_new(vec_insert_before_n)
        , test_case_new(vec_insert_before_en_invalid_magic)
        , test_case_new(vec_insert_before_en_invalid_elem)
        , test_case_new(vec_insert_before_en_no_member)
        , test_case_new(vec_insert_before_en_member_invalid_alignment)
        , test_case_new(vec_insert_before_en_nothing)
        , test_case_new(vec_insert_before_en)
        
        , test_case_new(vec_insert_after_invalid_magic)
        , test_case_new(vec_insert_after_invalid_elem)
        , test_case_new(vec_insert_after_no_member)
        , test_case_new(vec_insert_after_member_invalid_alignment)
        , test_case_new(vec_insert_after)
        , test_case_new(vec_insert_after_e_invalid_magic)
        , test_case_new(vec_insert_after_e_invalid_elem)
        , test_case_new(vec_insert_after_e_no_member)
        , test_case_new(vec_insert_after_e_member_invalid_alignment)
        , test_case_new(vec_insert_after_e)
        , test_case_new(vec_insert_after_p_invalid_magic)
        , test_case_new(vec_insert_after_p_invalid_elem)
        , test_case_new(vec_insert_after_p_invalid_type)
        , test_case_new(vec_insert_after_p_no_member)
        , test_case_new(vec_insert_after_p_member_invalid_alignment)
        , test_case_new(vec_insert_after_p)
        , test_case_new(vec_insert_after_n_invalid_magic)
        , test_case_new(vec_insert_after_n_invalid_elem)
        , test_case_new(vec_insert_after_n_no_member)
        , test_case_new(vec_insert_after_n_member_invalid_alignment)
        , test_case_new(vec_insert_after_n_nothing)
        , test_case_new(vec_insert_after_n)
        , test_case_new(vec_insert_after_en_invalid_magic)
        , test_case_new(vec_insert_after_en_invalid_elem)
        , test_case_new(vec_insert_after_en_no_member)
        , test_case_new(vec_insert_after_en_member_invalid_alignment)
        , test_case_new(vec_insert_after_en_nothing)
        , test_case_new(vec_insert_after_en)
        
        , test_case_new(vec_pop_invalid_magic)
        , test_case_new(vec_pop_empty_vector)
        , test_case_new(vec_pop)
        , test_case_new(vec_pop_e_invalid_magic)
        , test_case_new(vec_pop_e_empty_vector)
        , test_case_new(vec_pop_e)
        , test_case_new(vec_pop_p_invalid_magic)
        , test_case_new(vec_pop_p_invalid_type)
        , test_case_new(vec_pop_p_empty_vector)
        , test_case_new(vec_pop_p)
        , test_case_new(vec_pop_f_invalid_magic)
        , test_case_new(vec_pop_f_empty_vector)
        , test_case_new(vec_pop_f)
        , test_case_new(vec_pop_n_invalid_magic)
        , test_case_new(vec_pop_n_empty_vector)
        , test_case_new(vec_pop_n_nothing)
        , test_case_new(vec_pop_n)
        , test_case_new(vec_pop_n_fewer_members)
        , test_case_new(vec_pop_en_invalid_magic)
        , test_case_new(vec_pop_en_empty_vector)
        , test_case_new(vec_pop_en_nothing)
        , test_case_new(vec_pop_en)
        , test_case_new(vec_pop_en_fewer_members)
        , test_case_new(vec_pop_fn_invalid_magic)
        , test_case_new(vec_pop_fn_empty_vector)
        , test_case_new(vec_pop_fn_nothing)
        , test_case_new(vec_pop_fn)
        , test_case_new(vec_pop_fn_fewer_members)
        
        , test_case_new(vec_remove_invalid_magic)
        , test_case_new(vec_remove_invalid_elem)
        , test_case_new(vec_remove_no_member)
        , test_case_new(vec_remove_member_invalid_alignment)
        , test_case_new(vec_remove)
        , test_case_new(vec_remove_n_invalid_magic)
        , test_case_new(vec_remove_n_invalid_elem)
        , test_case_new(vec_remove_n_no_member)
        , test_case_new(vec_remove_n_member_invalid_alignment)
        , test_case_new(vec_remove_n_nothing)
        , test_case_new(vec_remove_n)
        , test_case_new(vec_remove_n_fewer_members)
        
        , test_case_new(vec_remove_at_invalid_magic)
        , test_case_new(vec_remove_at_oob_positive_index)
        , test_case_new(vec_remove_at_oob_negative_index)
        , test_case_new(vec_remove_at_positive_index)
        , test_case_new(vec_remove_at_negative_index)
        
        , test_case_new(vec_remove_at_e_invalid_magic)
        , test_case_new(vec_remove_at_e_oob_positive_index)
        , test_case_new(vec_remove_at_e_oob_negative_index)
        , test_case_new(vec_remove_at_e_positive_index)
        , test_case_new(vec_remove_at_e_negative_index)
        
        , test_case_new(vec_remove_at_p_invalid_magic)
        , test_case_new(vec_remove_at_p_invalid_type)
        , test_case_new(vec_remove_at_p_oob_positive_index)
        , test_case_new(vec_remove_at_p_oob_negative_index)
        , test_case_new(vec_remove_at_p_positive_index)
        , test_case_new(vec_remove_at_p_negative_index)
        
        , test_case_new(vec_remove_at_f_invalid_magic)
        , test_case_new(vec_remove_at_f_oob_positive_index)
        , test_case_new(vec_remove_at_f_oob_negative_index)
        , test_case_new(vec_remove_at_f_positive_index)
        , test_case_new(vec_remove_at_f_negative_index)
        
        , test_case_new(vec_remove_at_n_invalid_magic)
        , test_case_new(vec_remove_at_n_oob_positive_index)
        , test_case_new(vec_remove_at_n_oob_negative_index)
        , test_case_new(vec_remove_at_n_nothing_positive_index)
        , test_case_new(vec_remove_at_n_nothing_negative_index)
        , test_case_new(vec_remove_at_n_positive_index)
        , test_case_new(vec_remove_at_n_fewer_members_positive_index)
        , test_case_new(vec_remove_at_n_negative_index)
        , test_case_new(vec_remove_at_n_fewer_members_negative_index)
        
        , test_case_new(vec_remove_at_en_invalid_magic)
        , test_case_new(vec_remove_at_en_oob_positive_index)
        , test_case_new(vec_remove_at_en_oob_negative_index)
        , test_case_new(vec_remove_at_en_nothing_positive_index)
        , test_case_new(vec_remove_at_en_nothing_negative_index)
        , test_case_new(vec_remove_at_en_positive_index)
        , test_case_new(vec_remove_at_en_fewer_members_positive_index)
        , test_case_new(vec_remove_at_en_negative_index)
        , test_case_new(vec_remove_at_en_fewer_members_negative_index)
        
        , test_case_new(vec_remove_at_fn_invalid_magic)
        , test_case_new(vec_remove_at_fn_oob_positive_index)
        , test_case_new(vec_remove_at_fn_oob_negative_index)
        , test_case_new(vec_remove_at_fn_nothing_positive_index)
        , test_case_new(vec_remove_at_fn_nothing_negative_index)
        , test_case_new(vec_remove_at_fn_positive_index)
        , test_case_new(vec_remove_at_fn_fewer_members_positive_index)
        , test_case_new(vec_remove_at_fn_negative_index)
        , test_case_new(vec_remove_at_fn_fewer_members_negative_index)
        
        , test_case_new(vec_find_invalid_magic)
        , test_case_new(vec_find_invalid_pred)
        , test_case_new(vec_find_fail)
        , test_case_new(vec_find)
        , test_case_new(vec_find_p_invalid_magic)
        , test_case_new(vec_find_p_invalid_pred)
        , test_case_new(vec_find_p_invalid_type)
        , test_case_new(vec_find_p_fail)
        , test_case_new(vec_find_p)
        , test_case_new(vec_find_r_invalid_magic)
        , test_case_new(vec_find_r_invalid_pred)
        , test_case_new(vec_find_r_fail)
        , test_case_new(vec_find_r)
        , test_case_new(vec_find_rp_invalid_magic)
        , test_case_new(vec_find_rp_invalid_pred)
        , test_case_new(vec_find_rp_invalid_type)
        , test_case_new(vec_find_rp_fail)
        , test_case_new(vec_find_rp)
        
        , test_case_new(vec_find_pos_invalid_magic)
        , test_case_new(vec_find_pos_invalid_pred)
        , test_case_new(vec_find_pos_fail)
        , test_case_new(vec_find_pos)
        , test_case_new(vec_find_pos_r_invalid_magic)
        , test_case_new(vec_find_pos_r_invalid_pred)
        , test_case_new(vec_find_pos_r_fail)
        , test_case_new(vec_find_pos_r)
        
        , test_case_new(vec_find_get_invalid_magic)
        , test_case_new(vec_find_get_invalid_pred)
        , test_case_new(vec_find_get_fail)
        , test_case_new(vec_find_get)
        , test_case_new(vec_find_get_r_invalid_magic)
        , test_case_new(vec_find_get_r_invalid_pred)
        , test_case_new(vec_find_get_r_fail)
        , test_case_new(vec_find_get_r)
        
        , test_case_new(vec_find_remove_invalid_magic)
        , test_case_new(vec_find_remove_invalid_pred)
        , test_case_new(vec_find_remove_fail)
        , test_case_new(vec_find_remove)
        , test_case_new(vec_find_remove_p_invalid_magic)
        , test_case_new(vec_find_remove_p_invalid_pred)
        , test_case_new(vec_find_remove_p_invalid_type)
        , test_case_new(vec_find_remove_p_fail)
        , test_case_new(vec_find_remove_p)
        , test_case_new(vec_find_remove_f_invalid_magic)
        , test_case_new(vec_find_remove_f_invalid_pred)
        , test_case_new(vec_find_remove_f_fail)
        , test_case_new(vec_find_remove_f)
        , test_case_new(vec_find_remove_p)
        , test_case_new(vec_find_remove_r_invalid_magic)
        , test_case_new(vec_find_remove_r_invalid_pred)
        , test_case_new(vec_find_remove_r_fail)
        , test_case_new(vec_find_remove_r)
        , test_case_new(vec_find_remove_rp_invalid_magic)
        , test_case_new(vec_find_remove_rp_invalid_pred)
        , test_case_new(vec_find_remove_rp_invalid_type)
        , test_case_new(vec_find_remove_rp_fail)
        , test_case_new(vec_find_remove_rp)
        , test_case_new(vec_find_remove_rf_invalid_magic)
        , test_case_new(vec_find_remove_rf_invalid_pred)
        , test_case_new(vec_find_remove_rf_fail)
        , test_case_new(vec_find_remove_rf)
        , test_case_new(vec_find_remove_all_invalid_magic)
        , test_case_new(vec_find_remove_all_invalid_pred)
        , test_case_new(vec_find_remove_all)
        , test_case_new(vec_find_remove_all_f_invalid_magic)
        , test_case_new(vec_find_remove_all_f_invalid_pred)
        , test_case_new(vec_find_remove_all_f)
        
        , test_case_new(vec_swap_invalid_magic)
        , test_case_new(vec_swap_oob_positive_index)
        , test_case_new(vec_swap_oob_negative_index)
        , test_case_new(vec_swap_positive_index)
        , test_case_new(vec_swap_negative_index)
        
        , test_case_new(vec_get_buffer_invalid_magic)
        , test_case_new(vec_get_buffer_invalid_buffer)
        , test_case_new(vec_get_buffer_no_capacity)
        , test_case_new(vec_get_buffer)
        
        , test_case_new(vec_truncate_invalid_magic)
        , test_case_new(vec_truncate)
        , test_case_new(vec_truncate_f_invalid_magic)
        , test_case_new(vec_truncate_f)
        
        , test_case_new(vec_set_capacity_invalid_magic)
        , test_case_new(vec_set_capacity_below_min_capacity)
        , test_case_new(vec_set_capacity)
        , test_case_new(vec_set_capacity_below_current_capacity)
        , test_case_new(vec_set_capacity_f_invalid_magic)
        , test_case_new(vec_set_capacity_f_below_min_capacity)
        , test_case_new(vec_set_capacity_f)
        , test_case_new(vec_set_capacity_f_below_current_capacity)
        
        , test_case_new(vec_fold_invalid_magic)
        , test_case_new(vec_fold_invalid_fold)
        , test_case_new(vec_fold)
        , test_case_new(vec_fold_r_invalid_magic)
        , test_case_new(vec_fold_r_invalid_fold)
        , test_case_new(vec_fold_r)
    );
}
