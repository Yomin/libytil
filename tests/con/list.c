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
#include <ytil/test/test.h>
#include <ytil/con/list.h>

static const struct not_a_list
{
    int foo;
} not_a_list = { 123 }, not_a_node = { 123 };

static const int i[] = { 1, 2, 3, 4 };
static int count;
static list_ct list, list2;
static list_node_ct node;


TEST_SETUP(list_new)
{
    test_ptr_success(list = list_new());
    test_ptr_success(list_append_value(list, i[0]));
    test_ptr_success(list_append_value(list, i[1]));
    test_ptr_success(list_append_value(list, i[2]));
    test_ptr_success(list_append_value(list, i[3]));
}

TEST_SETUP(list_new_empty)
{
    test_ptr_success(list = list_new());
}

TEST_SETUP(list_new2)
{
    test_ptr_success(list = list_new());
    test_ptr_success(list_append_value(list, i[0]));
    test_ptr_success(list2 = list_new());
    test_ptr_success(list_append_value(list2, i[0]));
}

TEST_TEARDOWN(list_free)
{
    test_void(list_free(list));
}

TEST_TEARDOWN(list_free2)
{
    test_void(list_free(list));
    test_void(list_free(list2));
}

static void _test_list_dtor(list_const_ct list, void *data, void *ctx)
{
    int *count = ctx;

    count[0]++;
}

TEST_CASE_ABORT(list_free_invalid_magic)
{
    list_free((list_ct)&not_a_list);
}

TEST_CASE_ABORT(list_free_f_invalid_magic)
{
    list_free_f((list_ct)&not_a_list, _test_list_dtor, &count);
}

TEST_CASE_FIXTURE(list_free_f, list_new, NULL)
{
    count = 0;
    test_void(list_free_f(list, _test_list_dtor, &count));
    test_int_eq(count, 4);
}

TEST_CASE_ABORT(list_clear_invalid_magic)
{
    list_clear((list_ct)&not_a_list);
}

TEST_CASE_FIXTURE(list_clear, list_new, list_free)
{
    test_void(list_clear(list));
    test_uint_eq(list_size(list), 0);
}

TEST_CASE_ABORT(list_clear_f_invalid_magic)
{
    list_clear_f((list_ct)&not_a_list, _test_list_dtor, &count);
}

TEST_CASE_FIXTURE(list_clear_f, list_new, list_free)
{
    count = 0;
    test_void(list_clear_f(list, _test_list_dtor, &count));
    test_uint_eq(list_size(list), 0);
    test_int_eq(count, 4);
}

TEST_CASE_ABORT(list_clone_invalid_magic)
{
    list_clone((list_ct)&not_a_list);
}

TEST_CASE_FIXTURE(list_clone, list_new, list_free)
{
    test_ptr_success(list2 = list_clone(list));
    test_uint_eq(list_size(list), list_size(list2));
    test_int_eq(list_value_at(list2, 0, int), i[0]);
    test_int_eq(list_value_at(list2, 1, int), i[1]);
    test_int_eq(list_value_at(list2, 2, int), i[2]);
    test_int_eq(list_value_at(list2, 3, int), i[3]);
    list_free(list2);
}

static int _test_list_clone(list_const_ct list, void **dst, const void *src, void *ctx)
{
    *dst = VALUE_TO_POINTER(POINTER_TO_VALUE(src, int) + 1);

    return 0;
}

TEST_CASE_ABORT(list_clone_f_invalid_magic)
{
    list_clone_f((list_ct)&not_a_list, _test_list_clone, NULL, NULL);
}

TEST_CASE_FIXTURE(list_clone_f, list_new, list_free)
{
    test_ptr_success(list2 = list_clone_f(list, _test_list_clone, NULL, NULL));
    test_uint_eq(list_size(list), list_size(list2));
    test_int_eq(list_value_at(list2, 0, int), i[0] + 1);
    test_int_eq(list_value_at(list2, 1, int), i[1] + 1);
    test_int_eq(list_value_at(list2, 2, int), i[2] + 1);
    test_int_eq(list_value_at(list2, 3, int), i[3] + 1);
    list_free(list2);
}

TEST_CASE_ABORT(list_is_empty_invalid_magic)
{
    list_is_empty((list_ct)&not_a_list);
}

TEST_CASE_FIXTURE(list_is_empty, list_new_empty, list_free)
{
    test_true(list_is_empty(list));
    test_ptr_success(list_append_value(list, 123));
    test_false(list_is_empty(list));
    test_int_success(list_remove_at(list, 0));
    test_true(list_is_empty(list));
}

TEST_CASE_ABORT(list_size_invalid_magic)
{
    list_size((list_ct)&not_a_list);
}

TEST_CASE_FIXTURE(list_size, list_new_empty, list_free)
{
    test_uint_eq(list_size(list), 0);
    test_ptr_success(list_append_value(list, 123));
    test_uint_eq(list_size(list), 1);
    test_int_success(list_remove_at(list, 0));
    test_uint_eq(list_size(list), 0);
}

static size_t _test_list_memsize(list_const_ct list, const void *data, void *ctx)
{
    return 100;
}

TEST_CASE_ABORT(list_memsize_invalid_magic)
{
    list_memsize((list_ct)&not_a_list);
}

TEST_CASE_ABORT(list_memsize_f_invalid_magic)
{
    list_memsize_f((list_ct)&not_a_list, _test_list_memsize, NULL);
}

TEST_CASE_FIXTURE(list_memsize, list_new, list_free)
{
    test_uint_eq(list_memsize(list) + 400, list_memsize_f(list, _test_list_memsize, NULL));
}

TEST_CASE_ABORT(list_at_invalid_magic)
{
    list_at((list_ct)&not_a_list, 0);
}

TEST_CASE_FIXTURE(list_at_oob_positive_index, list_new, list_free)
{
    test_ptr_error(list_at(list, 4), E_LIST_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(list_at_oob_negative_index, list_new_empty, list_free)
{
    test_ptr_error(list_at(list, -5), E_LIST_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(list_at_positive_index, list_new, list_free)
{
    test_ptr_success(node = list_at(list, 3));
    test_int_eq(list_node_value(node, int), i[3]);
}

TEST_CASE_FIXTURE(list_at_negative_index, list_new, list_free)
{
    test_ptr_success(node = list_at(list, -4));
    test_int_eq(list_node_value(node, int), i[0]);
}

TEST_CASE_ABORT(list_first_invalid_magic)
{
    list_first((list_ct)&not_a_list);
}

TEST_CASE_FIXTURE(list_first_empty_list, list_new_empty, list_free)
{
    test_ptr_error(list_first(list), E_LIST_EMPTY);
}

TEST_CASE_FIXTURE(list_first, list_new, list_free)
{
    test_ptr_success(node = list_first(list));
    test_int_eq(list_node_value(node, int), i[0]);
}

TEST_CASE_ABORT(list_last_invalid_magic)
{
    list_last((list_ct)&not_a_list);
}

TEST_CASE_FIXTURE(list_last_empty_list, list_new_empty, list_free)
{
    test_ptr_error(list_last(list), E_LIST_EMPTY);
}

TEST_CASE_FIXTURE(list_last, list_new, list_free)
{
    test_ptr_success(node = list_last(list));
    test_int_eq(list_node_value(node, int), i[3]);
}

TEST_CASE_ABORT_FIXTURE(list_next_invalid_list_magic, list_new, list_free)
{
    list_next((list_ct)&not_a_list, list_first(list));
}

TEST_CASE_ABORT_FIXTURE(list_next_invalid_node_magic, list_new, list_free)
{
    list_next(list, (list_node_ct)&not_a_node);
}

TEST_CASE_ABORT_FIXTURE(list_next_node_not_member, list_new2, list_free2)
{
    list_next(list, list_first(list2));
}

TEST_CASE_FIXTURE(list_next, list_new, list_free)
{
    test_ptr_success(node = list_next(list, list_first(list)));
    test_int_eq(list_node_value(node, int), i[1]);
}

TEST_CASE_FIXTURE(list_next_end, list_new, list_free)
{
    test_ptr_error(list_next(list, list_last(list)), E_LIST_END);
}

TEST_CASE_ABORT_FIXTURE(list_prev_invalid_list_magic, list_new, list_free)
{
    list_prev((list_ct)&not_a_list, list_last(list));
}

TEST_CASE_ABORT_FIXTURE(list_prev_invalid_node_magic, list_new, list_free)
{
    list_prev(list, (list_node_ct)&not_a_node);
}

TEST_CASE_ABORT_FIXTURE(list_prev_node_not_member, list_new2, list_free2)
{
    list_prev(list, list_first(list2));
}

TEST_CASE_FIXTURE(list_prev, list_new, list_free)
{
    test_ptr_success(node = list_prev(list, list_last(list)));
    test_int_eq(list_node_value(node, int), i[2]);
}

TEST_CASE_FIXTURE(list_prev_end, list_new, list_free)
{
    test_ptr_error(list_prev(list, list_first(list)), E_LIST_END);
}

TEST_CASE_ABORT(list_data_at_invalid_magic)
{
    list_data_at((list_ct)&not_a_list, 0);
}

TEST_CASE_FIXTURE(list_data_at_oob_positive_index, list_new_empty, list_free)
{
    test_ptr_error(list_data_at(list, 3), E_LIST_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(list_data_at_oob_negative_index, list_new_empty, list_free)
{
    test_ptr_error(list_data_at(list, -5), E_LIST_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(list_data_at_positive_index, list_new, list_free)
{
    test_rc_success(list_value_at(list, 3, int), i[3]);
}

TEST_CASE_FIXTURE(list_data_at_negative_index, list_new, list_free)
{
    test_rc_success(list_value_at(list, -4, int), i[0]);
}

TEST_CASE_ABORT(list_data_first_invalid_magic)
{
    list_data_first((list_ct)&not_a_list);
}

TEST_CASE_FIXTURE(list_data_first_empty_list, list_new_empty, list_free)
{
    test_ptr_error(list_data_first(list), E_LIST_EMPTY);
}

TEST_CASE_FIXTURE(list_data_first, list_new, list_free)
{
    test_rc_success(list_value_first(list, int), i[0]);
}

TEST_CASE_ABORT(list_data_last_invalid_magic)
{
    list_data_last((list_ct)&not_a_list);
}

TEST_CASE_FIXTURE(list_data_last_empty_list, list_new_empty, list_free)
{
    test_ptr_error(list_data_last(list), E_LIST_EMPTY);
}

TEST_CASE_FIXTURE(list_data_last, list_new, list_free)
{
    test_rc_success(list_value_last(list, int), i[3]);
}

TEST_CASE_ABORT_FIXTURE(list_pos_invalid_list_magic, list_new, list_free)
{
    list_pos((list_ct)&not_a_list, list_first(list));
}

TEST_CASE_ABORT_FIXTURE(list_pos_invalid_node_magic, list_new, list_free)
{
    list_pos(list, (list_node_ct)&not_a_node);
}

TEST_CASE_ABORT_FIXTURE(list_pos_node_not_member, list_new2, list_free2)
{
    list_pos(list, list_first(list2));
}

TEST_CASE_FIXTURE(list_pos, list_new, list_free)
{
    test_rc_success(list_pos(list, list_next(list, list_first(list))), 1);
}

TEST_CASE_ABORT(list_prepend_invalid_magic)
{
    list_prepend((list_ct)&not_a_list, NULL);
}

TEST_CASE_FIXTURE(list_prepend, list_new, list_free)
{
    test_ptr_success(node = list_prepend_value(list, 42));
    test_uint_eq(list_pos(list, node), 0);
    test_uint_eq(list_size(list), 5);
    test_int_eq(list_node_value(node, int), 42);
}

TEST_CASE_ABORT(list_append_invalid_magic)
{
    list_append((list_ct)&not_a_list, NULL);
}

TEST_CASE_FIXTURE(list_append, list_new, list_free)
{
    test_ptr_success(node = list_append_value(list, 42));
    test_uint_eq(list_pos(list, node), 4);
    test_uint_eq(list_size(list), 5);
    test_int_eq(list_node_value(node, int), 42);
}

TEST_CASE_ABORT(list_insert_invalid_magic)
{
    list_insert((list_ct)&not_a_list, 0, NULL);
}

TEST_CASE_FIXTURE(list_insert_oob_positive_index, list_new, list_free)
{
    test_ptr_error(list_insert(list, 5, NULL), E_LIST_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(list_insert_head_positive_index, list_new, list_free)
{
    test_ptr_success(node = list_insert_value(list, 0, 42));
    test_uint_eq(list_pos(list, node), 0);
    test_uint_eq(list_size(list), 5);
    test_int_eq(list_node_value(node, int), 42);
}

TEST_CASE_FIXTURE(list_insert_tail_positive_index, list_new, list_free)
{
    test_ptr_success(node = list_insert_value(list, 4, 42));
    test_uint_eq(list_pos(list, node), 4);
    test_uint_eq(list_size(list), 5);
    test_int_eq(list_node_value(node, int), 42);
}

TEST_CASE_FIXTURE(list_insert_oob_negative_index, list_new, list_free)
{
    test_ptr_error(list_insert(list, -5, NULL), E_LIST_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(list_insert_head_negative_index, list_new, list_free)
{
    test_ptr_success(node = list_insert_value(list, -4, 42));
    test_uint_eq(list_pos(list, node), 0);
    test_uint_eq(list_size(list), 5);
    test_int_eq(list_node_value(node, int), 42);
}

TEST_CASE_FIXTURE(list_insert_tail_negative_index, list_new, list_free)
{
    test_ptr_success(node = list_insert_value(list, -1, 42));
    test_uint_eq(list_pos(list, node), 3);
    test_uint_eq(list_size(list), 5);
    test_int_eq(list_node_value(node, int), 42);
}

TEST_CASE_ABORT_FIXTURE(list_insert_before_invalid_list_magic, list_new2, list_free2)
{
    list_insert_before((list_ct)&not_a_list, list_first(list2), NULL);
}

TEST_CASE_ABORT_FIXTURE(list_insert_before_invalid_node_magic, list_new2, list_free2)
{
    list_insert_before(list, (list_node_ct)&not_a_node, NULL);
}

TEST_CASE_ABORT_FIXTURE(list_insert_before_node_not_member, list_new2, list_free2)
{
    list_insert_before(list, list_first(list2), NULL);
}

TEST_CASE_FIXTURE(list_insert_before, list_new, list_free)
{
    test_ptr_success(node = list_insert_value_before(list, list_first(list), 42));
    test_uint_eq(list_pos(list, node), 0);
    test_uint_eq(list_size(list), 5);
    test_int_eq(list_node_value(node, int), 42);
}

TEST_CASE_ABORT_FIXTURE(list_insert_after_invalid_list_magic, list_new2, list_free2)
{
    list_insert_after((list_ct)&not_a_list, list_first(list2), NULL);
}

TEST_CASE_ABORT_FIXTURE(list_insert_after_invalid_node_magic, list_new2, list_free2)
{
    list_insert_after(list, (list_node_ct)&not_a_node, NULL);
}

TEST_CASE_ABORT_FIXTURE(list_insert_after_node_not_member, list_new2, list_free2)
{
    list_insert_after(list, list_first(list2), NULL);
}

TEST_CASE_FIXTURE(list_insert_after, list_new, list_free)
{
    test_ptr_success(node = list_insert_value_after(list, list_last(list), 42));
    test_uint_eq(list_pos(list, node), 4);
    test_uint_eq(list_size(list), 5);
    test_int_eq(list_node_value(node, int), 42);
}

TEST_CASE_ABORT_FIXTURE(list_remove_invalid_list_magic, list_new2, list_free2)
{
    list_remove((list_ct)&not_a_list, list_first(list2));
}

TEST_CASE_ABORT_FIXTURE(list_remove_invalid_node_magic, list_new2, list_free2)
{
    list_remove(list, (list_node_ct)&not_a_node);
}

TEST_CASE_ABORT_FIXTURE(list_remove_node_not_member, list_new2, list_free2)
{
    list_remove(list, list_first(list2));
}

TEST_CASE_FIXTURE(list_remove_head, list_new, list_free)
{
    test_void(list_remove(list, list_first(list)));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_first(list, int), 2);
}

TEST_CASE_FIXTURE(list_remove_tail, list_new, list_free)
{
    test_void(list_remove(list, list_last(list)));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_last(list, int), 3);
}

TEST_CASE_ABORT(list_remove_at_invalid_magic)
{
    list_remove_at((list_ct)&not_a_list, 0);
}

TEST_CASE_FIXTURE(list_remove_at_oob_positive_index, list_new, list_free)
{
    test_int_error(list_remove_at(list, 4), E_LIST_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(list_remove_at_oob_negative_index, list_new, list_free)
{
    test_int_error(list_remove_at(list, -5), E_LIST_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(list_remove_at_head_positive_index, list_new, list_free)
{
    test_int_success(list_remove_at(list, 0));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_first(list, int), 2);
}

TEST_CASE_FIXTURE(list_remove_at_tail_positive_index, list_new, list_free)
{
    test_int_success(list_remove_at(list, 3));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_last(list, int), 3);
}

TEST_CASE_FIXTURE(list_remove_at_head_negative_index, list_new, list_free)
{
    test_int_success(list_remove_at(list, -4));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_first(list, int), 2);
}

TEST_CASE_FIXTURE(list_remove_at_tail_negative_index, list_new, list_free)
{
    test_int_success(list_remove_at(list, -1));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_last(list, int), 3);
}

TEST_CASE_ABORT(list_remove_at_f_invalid_magic)
{
    list_remove_at_f((list_ct)&not_a_list, 0, _test_list_dtor, &count);
}

TEST_CASE_FIXTURE(list_remove_at_f_oob_positive_index, list_new, list_free)
{
    test_int_error(list_remove_at_f(list, 4, _test_list_dtor, &count), E_LIST_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(list_remove_at_f_oob_negative_index, list_new, list_free)
{
    test_int_error(list_remove_at_f(list, -5, _test_list_dtor, &count), E_LIST_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(list_remove_at_f_head_positive_index, list_new, list_free)
{
    count = 0;
    test_int_success(list_remove_at_f(list, 0, _test_list_dtor, &count));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_first(list, int), 2);
    test_int_eq(count, 1);
}

TEST_CASE_FIXTURE(list_remove_at_f_tail_positive_index, list_new, list_free)
{
    count = 0;
    test_int_success(list_remove_at_f(list, 3, _test_list_dtor, &count));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_last(list, int), 3);
    test_int_eq(count, 1);
}

TEST_CASE_FIXTURE(list_remove_at_f_head_negative_index, list_new, list_free)
{
    count = 0;
    test_int_success(list_remove_at_f(list, -4, _test_list_dtor, &count));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_first(list, int), 2);
    test_int_eq(count, 1);
}

TEST_CASE_FIXTURE(list_remove_at_f_tail_negative_index, list_new, list_free)
{
    count = 0;
    test_int_success(list_remove_at_f(list, -1, _test_list_dtor, &count));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_last(list, int), 3);
    test_int_eq(count, 1);
}

static bool _test_list_pred(list_const_ct list, const void *data, void *ctx)
{
    return POINTER_TO_VALUE(data, int) % 2;
}

TEST_CASE_ABORT(list_find_invalid_magic)
{
    list_find((list_ct)&not_a_list, _test_list_pred, NULL);
}

TEST_CASE_ABORT_FIXTURE(list_find_invalid_pred, list_new, list_free)
{
    list_find(list, NULL, NULL);
}

TEST_CASE_FIXTURE(list_find_not_found, list_new_empty, list_free)
{
    test_ptr_error(list_find(list, _test_list_pred, NULL), E_LIST_NOT_FOUND);
}

TEST_CASE_FIXTURE(list_find, list_new, list_free)
{
    test_ptr_success(node = list_find(list, _test_list_pred, NULL));
    test_uint_eq(list_pos(list, node), 0);
}

TEST_CASE_ABORT(list_find_r_invalid_magic)
{
    list_find_r((list_ct)&not_a_list, _test_list_pred, NULL);
}

TEST_CASE_ABORT_FIXTURE(list_find_r_invalid_pred, list_new, list_free)
{
    list_find_r(list, NULL, NULL);
}

TEST_CASE_FIXTURE(list_find_r_not_found, list_new_empty, list_free)
{
    test_ptr_error(list_find_r(list, _test_list_pred, NULL), E_LIST_NOT_FOUND);
}

TEST_CASE_FIXTURE(list_find_r, list_new, list_free)
{
    test_ptr_success(node = list_find_r(list, _test_list_pred, NULL));
    test_uint_eq(list_pos(list, node), 2);
}

TEST_CASE_ABORT(list_find_remove_invalid_magic)
{
    list_find_remove((list_ct)&not_a_list, _test_list_pred, NULL);
}

TEST_CASE_ABORT_FIXTURE(list_find_remove_invalid_pred, list_new, list_free)
{
    list_find_remove(list, NULL, NULL);
}

TEST_CASE_FIXTURE(list_find_remove_not_found, list_new_empty, list_free)
{
    test_int_error(list_find_remove(list, _test_list_pred, NULL), E_LIST_NOT_FOUND);
}

TEST_CASE_FIXTURE(list_find_remove, list_new, list_free)
{
    test_int_success(list_find_remove(list, _test_list_pred, NULL));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_first(list, int), 2);
}

TEST_CASE_ABORT(list_find_remove_f_invalid_magic)
{
    list_find_remove_f((list_ct)&not_a_list, _test_list_pred, NULL, _test_list_dtor, &count);
}

TEST_CASE_ABORT_FIXTURE(list_find_remove_f_invalid_pred, list_new, list_free)
{
    list_find_remove_f(list, NULL, NULL, _test_list_dtor, &count);
}

TEST_CASE_FIXTURE(list_find_remove_f_not_found, list_new_empty, list_free)
{
    test_int_error(list_find_remove_f(list, _test_list_pred, NULL, _test_list_dtor, &count), E_LIST_NOT_FOUND);
}

TEST_CASE_FIXTURE(list_find_remove_f, list_new, list_free)
{
    count = 0;
    test_int_success(list_find_remove_f(list, _test_list_pred, NULL, _test_list_dtor, &count));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_first(list, int), 2);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(list_find_remove_r_invalid_magic)
{
    list_find_remove_r((list_ct)&not_a_list, _test_list_pred, NULL);
}

TEST_CASE_ABORT_FIXTURE(list_find_remove_r_invalid_pred, list_new, list_free)
{
    list_find_remove_r(list, NULL, NULL);
}

TEST_CASE_FIXTURE(list_find_remove_r_not_found, list_new_empty, list_free)
{
    test_int_error(list_find_remove_r(list, _test_list_pred, NULL), E_LIST_NOT_FOUND);
}

TEST_CASE_FIXTURE(list_find_remove_r, list_new, list_free)
{
    test_int_success(list_find_remove_r(list, _test_list_pred, NULL));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_at(list, 2, int), 4);
}

TEST_CASE_ABORT(list_find_remove_rf_invalid_magic)
{
    list_find_remove_rf((list_ct)&not_a_list, _test_list_pred, NULL, _test_list_dtor, &count);
}

TEST_CASE_ABORT_FIXTURE(list_find_remove_rf_invalid_pred, list_new, list_free)
{
    list_find_remove_rf(list, NULL, NULL, _test_list_dtor, &count);
}

TEST_CASE_FIXTURE(list_find_remove_rf_not_found, list_new_empty, list_free)
{
    test_int_error(list_find_remove_rf(list, _test_list_pred, NULL, _test_list_dtor, &count), E_LIST_NOT_FOUND);
}

TEST_CASE_FIXTURE(list_find_remove_rf, list_new, list_free)
{
    count = 0;
    test_int_success(list_find_remove_rf(list, _test_list_pred, NULL, _test_list_dtor, &count));
    test_uint_eq(list_size(list), 3);
    test_int_eq(list_value_at(list, 2, int), 4);
    test_int_eq(count, 1);
}

TEST_CASE_ABORT(list_find_remove_all_invalid_magic)
{
    list_find_remove_all((list_ct)&not_a_list, _test_list_pred, NULL);
}

TEST_CASE_ABORT_FIXTURE(list_find_remove_all_invalid_pred, list_new, list_free)
{
    list_find_remove_all(list, NULL, NULL);
}

TEST_CASE_FIXTURE(list_find_remove_all_not_found, list_new_empty, list_free)
{
    test_uint_eq(list_find_remove_all(list, _test_list_pred, NULL), 0);
}

TEST_CASE_FIXTURE(list_find_remove_all, list_new, list_free)
{
    test_uint_eq(list_find_remove_all(list, _test_list_pred, NULL), 2);
    test_uint_eq(list_size(list), 2);
    test_int_eq(list_value_at(list, 0, int), 2);
    test_int_eq(list_value_at(list, 1, int), 4);
}

TEST_CASE_ABORT(list_find_remove_all_f_invalid_magic)
{
    list_find_remove_all_f((list_ct)&not_a_list, _test_list_pred, NULL, _test_list_dtor, &count);
}

TEST_CASE_ABORT_FIXTURE(list_find_remove_all_f_invalid_pred, list_new, list_free)
{
    list_find_remove_all_f(list, NULL, NULL, _test_list_dtor, &count);
}

TEST_CASE_FIXTURE(list_find_remove_all_f_not_found, list_new_empty, list_free)
{
    test_uint_eq(list_find_remove_all_f(list, _test_list_pred, NULL, _test_list_dtor, &count), 0);
}

TEST_CASE_FIXTURE(list_find_remove_all_f, list_new, list_free)
{
    count = 0;
    test_uint_eq(list_find_remove_all_f(list, _test_list_pred, NULL, _test_list_dtor, &count), 2);
    test_uint_eq(list_size(list), 2);
    test_int_eq(list_value_at(list, 0, int), 2);
    test_int_eq(list_value_at(list, 1, int), 4);
    test_int_eq(count, 2);
}

TEST_CASE_ABORT_FIXTURE(list_swap_invalid_node1_magic, list_new2, list_free2)
{
    list_swap((list_node_ct)&not_a_node, list_first(list2));
}

TEST_CASE_ABORT_FIXTURE(list_swap_invalid_node2_magic, list_new2, list_free2)
{
    list_swap(list_first(list), (list_node_ct)&not_a_node);
}

TEST_CASE_ABORT_FIXTURE(list_swap_nodes_in_different_lists, list_new2, list_free2)
{
    list_swap(list_first(list), list_first(list2));
}

TEST_CASE_FIXTURE(list_swap, list_new, list_free)
{
    test_void(list_swap(list_first(list), list_last(list)));
    test_int_eq(list_value_at(list, 0, int), 4);
    test_int_eq(list_value_at(list, 1, int), 2);
    test_int_eq(list_value_at(list, 2, int), 3);
    test_int_eq(list_value_at(list, 3, int), 1);
}

static int _test_list_fold(list_const_ct list, void *data, void *ctx)
{
    int *sum = ctx;

    *sum = *sum * 10 + POINTER_TO_VALUE(data, int);

    return 0;
}

TEST_CASE_ABORT(list_fold_invalid_magic)
{
    list_fold((list_ct)&not_a_list, _test_list_fold, NULL);
}

TEST_CASE_ABORT_FIXTURE(list_fold_invalid_callback, list_new, list_free)
{
    list_fold(list, NULL, NULL);
}

TEST_CASE_FIXTURE(list_fold, list_new, list_free)
{
    int sum = 0;
    test_int_success(list_fold(list, _test_list_fold, &sum));
    test_int_eq(sum, 1234);
}

TEST_CASE_ABORT(list_fold_r_invalid_magic)
{
    list_fold_r((list_ct)&not_a_list, _test_list_fold, NULL);
}

TEST_CASE_ABORT_FIXTURE(list_fold_r_invalid_callback, list_new, list_free)
{
    list_fold_r(list, NULL, NULL);
}

TEST_CASE_FIXTURE(list_fold_r, list_new, list_free)
{
    int sum = 0;
    test_int_success(list_fold_r(list, _test_list_fold, &sum));
    test_int_eq(sum, 4321);
}

test_suite_ct test_suite_con_list(void)
{
    return test_suite_new_with_cases("list",
        test_case_new(list_free_invalid_magic),
        test_case_new(list_free_f_invalid_magic),
        test_case_new(list_free_f),

        test_case_new(list_clear_invalid_magic),
        test_case_new(list_clear),
        test_case_new(list_clear_f_invalid_magic),
        test_case_new(list_clear_f),

        test_case_new(list_clone_invalid_magic),
        test_case_new(list_clone),
        test_case_new(list_clone_f_invalid_magic),
        test_case_new(list_clone_f),

        test_case_new(list_is_empty_invalid_magic),
        test_case_new(list_is_empty),
        test_case_new(list_size_invalid_magic),
        test_case_new(list_size),
        test_case_new(list_memsize_invalid_magic),
        test_case_new(list_memsize_f_invalid_magic),
        test_case_new(list_memsize),

        test_case_new(list_at_invalid_magic),
        test_case_new(list_at_oob_positive_index),
        test_case_new(list_at_oob_negative_index),
        test_case_new(list_at_positive_index),
        test_case_new(list_at_negative_index),

        test_case_new(list_first_invalid_magic),
        test_case_new(list_first_empty_list),
        test_case_new(list_first),
        test_case_new(list_last_invalid_magic),
        test_case_new(list_last_empty_list),
        test_case_new(list_last),

        test_case_new(list_next_invalid_list_magic),
        test_case_new(list_next_invalid_node_magic),
        test_case_new(list_next_node_not_member),
        test_case_new(list_next),
        test_case_new(list_next_end),
        test_case_new(list_prev_invalid_list_magic),
        test_case_new(list_prev_invalid_node_magic),
        test_case_new(list_prev_node_not_member),
        test_case_new(list_prev),
        test_case_new(list_prev_end),

        test_case_new(list_data_at_invalid_magic),
        test_case_new(list_data_at_oob_positive_index),
        test_case_new(list_data_at_oob_negative_index),
        test_case_new(list_data_at_positive_index),
        test_case_new(list_data_at_negative_index),

        test_case_new(list_data_first_invalid_magic),
        test_case_new(list_data_first_empty_list),
        test_case_new(list_data_first),
        test_case_new(list_data_last_invalid_magic),
        test_case_new(list_data_last_empty_list),
        test_case_new(list_data_last),

        test_case_new(list_pos_invalid_list_magic),
        test_case_new(list_pos_invalid_node_magic),
        test_case_new(list_pos_node_not_member),
        test_case_new(list_pos),

        test_case_new(list_prepend_invalid_magic),
        test_case_new(list_prepend),
        test_case_new(list_append_invalid_magic),
        test_case_new(list_append),

        test_case_new(list_insert_invalid_magic),
        test_case_new(list_insert_oob_positive_index),
        test_case_new(list_insert_head_positive_index),
        test_case_new(list_insert_tail_positive_index),
        test_case_new(list_insert_oob_negative_index),
        test_case_new(list_insert_head_negative_index),
        test_case_new(list_insert_tail_negative_index),

        test_case_new(list_insert_before_invalid_list_magic),
        test_case_new(list_insert_before_invalid_node_magic),
        test_case_new(list_insert_before_node_not_member),
        test_case_new(list_insert_before),
        test_case_new(list_insert_after_invalid_list_magic),
        test_case_new(list_insert_after_invalid_node_magic),
        test_case_new(list_insert_after_node_not_member),
        test_case_new(list_insert_after),

        test_case_new(list_remove_invalid_list_magic),
        test_case_new(list_remove_invalid_node_magic),
        test_case_new(list_remove_node_not_member),
        test_case_new(list_remove_head),
        test_case_new(list_remove_tail),

        test_case_new(list_remove_at_invalid_magic),
        test_case_new(list_remove_at_oob_positive_index),
        test_case_new(list_remove_at_head_positive_index),
        test_case_new(list_remove_at_tail_positive_index),
        test_case_new(list_remove_at_oob_negative_index),
        test_case_new(list_remove_at_head_negative_index),
        test_case_new(list_remove_at_tail_negative_index),

        test_case_new(list_remove_at_f_invalid_magic),
        test_case_new(list_remove_at_f_oob_positive_index),
        test_case_new(list_remove_at_f_head_positive_index),
        test_case_new(list_remove_at_f_tail_positive_index),
        test_case_new(list_remove_at_f_oob_negative_index),
        test_case_new(list_remove_at_f_head_negative_index),
        test_case_new(list_remove_at_f_tail_negative_index),

        test_case_new(list_find_invalid_magic),
        test_case_new(list_find_invalid_pred),
        test_case_new(list_find_not_found),
        test_case_new(list_find),
        test_case_new(list_find_r_invalid_magic),
        test_case_new(list_find_r_invalid_pred),
        test_case_new(list_find_r_not_found),
        test_case_new(list_find_r),

        test_case_new(list_find_remove_invalid_magic),
        test_case_new(list_find_remove_invalid_pred),
        test_case_new(list_find_remove_not_found),
        test_case_new(list_find_remove),
        test_case_new(list_find_remove_f_invalid_magic),
        test_case_new(list_find_remove_f_invalid_pred),
        test_case_new(list_find_remove_f_not_found),
        test_case_new(list_find_remove_f),

        test_case_new(list_find_remove_r_invalid_magic),
        test_case_new(list_find_remove_r_invalid_pred),
        test_case_new(list_find_remove_r_not_found),
        test_case_new(list_find_remove_r),
        test_case_new(list_find_remove_rf_invalid_magic),
        test_case_new(list_find_remove_rf_invalid_pred),
        test_case_new(list_find_remove_rf_not_found),
        test_case_new(list_find_remove_rf),

        test_case_new(list_find_remove_all_invalid_magic),
        test_case_new(list_find_remove_all_invalid_pred),
        test_case_new(list_find_remove_all_not_found),
        test_case_new(list_find_remove_all),
        test_case_new(list_find_remove_all_f_invalid_magic),
        test_case_new(list_find_remove_all_f_invalid_pred),
        test_case_new(list_find_remove_all_f_not_found),
        test_case_new(list_find_remove_all_f),

        test_case_new(list_swap_invalid_node1_magic),
        test_case_new(list_swap_invalid_node2_magic),
        test_case_new(list_swap_nodes_in_different_lists),
        test_case_new(list_swap),

        test_case_new(list_fold_invalid_magic),
        test_case_new(list_fold_invalid_callback),
        test_case_new(list_fold),
        test_case_new(list_fold_r_invalid_magic),
        test_case_new(list_fold_r_invalid_callback),
        test_case_new(list_fold_r)
    );
}
