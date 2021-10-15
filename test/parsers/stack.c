/*
 * Copyright (c) 2021 Martin Rödel a.k.a. Yomin Nimoy
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

#include "parsers.h"
#include <ytil/test/run.h>
#include <ytil/test/test.h>
#include <ytil/parser/stack.h>


static const struct not_a_stack
{
    int foo;
} not_a_stack = { 123 };

static parser_stack_ct stack;
static int sum;


TEST_SETUP(parser_stack_new)
{
    test_ptr_success(stack = parser_stack_new());
}

TEST_TEARDOWN(parser_stack_free)
{
    test_void(parser_stack_free(stack));
}

TEST_CASE_ABORT(parser_stack_free_invalid_magic)
{
    parser_stack_free((parser_stack_ct)&not_a_stack);
}

TEST_CASE_FIX(parser_stack_new_free, parser_stack_new, parser_stack_free)
{

}

static void test_parser_stack_dtor(void *item)
{
    int *i = item;

    sum += *i;
}

TEST_CASE_FIX(parser_stack_free_dtor, parser_stack_new, no_teardown)
{
    int i1 = 1, i2 = 123, i3 = 2;

    test_int_success(parser_stack_push(stack, "int", &i1, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_push(stack, "int", &i2, sizeof(int), NULL));
    test_int_success(parser_stack_push(stack, "int", &i3, sizeof(int), test_parser_stack_dtor));

    sum = 0;
    test_void(parser_stack_free(stack));
    test_int_eq(sum, i1 + i3);
}

TEST_CASE_FIX(parser_stack_free_skip_arg, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_push_p(stack, "type", "bar", NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push_p(stack, "type", "baz", NULL));
    test_int_success(parser_stack_pop_arg(stack, "type", NULL));
}

TEST_CASE_FIX(parser_stack_free_dtor_skip_arg, parser_stack_new, no_teardown)
{
    int i1 = 1, i2 = 123, i3 = 2;

    test_int_success(parser_stack_push(stack, "int", &i1, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_push(stack, "int", &i2, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push(stack, "int", &i3, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_pop_arg(stack, "int", NULL));

    sum = 0;
    test_void(parser_stack_free(stack));
    test_int_eq(sum, i1 + i3);
}

TEST_CASE_ABORT(parser_stack_push_invalid_magic)
{
    parser_stack_push((parser_stack_ct)&not_a_stack, "type", "data", 5, NULL);
}

TEST_CASE_FIX_ABORT(parser_stack_push_invalid_type, parser_stack_new, no_teardown)
{
    parser_stack_push(stack, NULL, "data", 5, NULL);
}

TEST_CASE_FIX_ABORT(parser_stack_push_invalid_data, parser_stack_new, no_teardown)
{
    parser_stack_push(stack, "type", NULL, 1, NULL);
}

TEST_CASE_FIX(parser_stack_push, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push(stack, "type", "foo", 4, NULL));
    test_uint_eq(parser_stack_size(stack), 1);
    test_str_eq(parser_stack_get_type(stack, 0), "type");
    test_uint_eq(parser_stack_get_size(stack, 0), 4);
    test_str_eq(parser_stack_at(stack, "type", 0), "foo");
}

TEST_CASE_FIX(parser_stack_push_null, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push(stack, "type", NULL, 0, NULL));
    test_uint_eq(parser_stack_size(stack), 1);
    test_str_eq(parser_stack_get_type(stack, 0), "type");
    test_uint_eq(parser_stack_get_size(stack, 0), 0);
}

TEST_CASE_ABORT(parser_stack_push_p_invalid_magic)
{
    parser_stack_push_p((parser_stack_ct)&not_a_stack, "type", "ptr", NULL);
}

TEST_CASE_FIX_ABORT(parser_stack_push_p_invalid_type, parser_stack_new, no_teardown)
{
    parser_stack_push_p(stack, NULL, "ptr", NULL);
}

TEST_CASE_FIX(parser_stack_push_p, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_uint_eq(parser_stack_size(stack), 1);
    test_str_eq(parser_stack_get_type(stack, 0), "type");
    test_uint_eq(parser_stack_get_size(stack, 0), sizeof(char *));
    test_str_eq(*(char **)parser_stack_at(stack, "type", 0), "foo");
}

TEST_CASE_FIX(parser_stack_push_p_null, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", NULL, NULL));
    test_uint_eq(parser_stack_size(stack), 1);
    test_str_eq(parser_stack_get_type(stack, 0), "type");
    test_uint_eq(parser_stack_get_size(stack, 0), sizeof(char *));
    test_ptr_eq(*(void **)parser_stack_at(stack, "type", 0), NULL);
}

TEST_CASE_ABORT(parser_stack_pop_invalid_magic)
{
    parser_stack_pop((parser_stack_ct)&not_a_stack, "type", NULL);
}

TEST_CASE_FIX_ABORT(parser_stack_pop_invalid_type, parser_stack_new, no_teardown)
{
    parser_stack_pop(stack, NULL, NULL);
}

TEST_CASE_FIX(parser_stack_pop_empty, parser_stack_new, parser_stack_free)
{
    test_int_error(parser_stack_pop(stack, "type", NULL), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_pop, parser_stack_new, parser_stack_free)
{
    char foo[4];

    test_int_success(parser_stack_push(stack, "type", "foo", 4, NULL));
    test_int_success(parser_stack_pop(stack, "type", foo));
    test_uint_eq(parser_stack_size(stack), 0);
    test_str_eq(foo, "foo");
}

TEST_CASE_FIX(parser_stack_pop_null, parser_stack_new, parser_stack_free)
{
    char foo[4] = "bar";

    test_int_success(parser_stack_push(stack, "type", NULL, 0, NULL));
    test_int_success(parser_stack_pop(stack, "type", foo));
    test_uint_eq(parser_stack_size(stack), 0);
    test_str_eq(foo, "bar");
}

TEST_CASE_FIX(parser_stack_pop_dtor, parser_stack_new, parser_stack_free)
{
    int i = 123;

    sum = 0;
    test_int_success(parser_stack_push(stack, "int", &i, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_pop(stack, "int", NULL));
    test_uint_eq(parser_stack_size(stack), 0);
    test_int_eq(sum, i);
}

TEST_CASE_FIX(parser_stack_pop_skip_arg, parser_stack_new, parser_stack_free)
{
    char foo[4];

    test_int_success(parser_stack_push(stack, "type", "foo", 4, NULL));
    test_int_success(parser_stack_push(stack, "type", "bar", 4, NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_pop_arg(stack, "type", NULL));
    test_int_success(parser_stack_frame_pop(stack));
    test_int_success(parser_stack_pop(stack, "type", foo));
    test_uint_eq(parser_stack_size(stack), 0);
    test_str_eq(foo, "foo");
}

TEST_CASE_FIX(parser_stack_pop_dtor_skip_arg, parser_stack_new, parser_stack_free)
{
    int i1 = 1, i2 = 123;

    test_int_success(parser_stack_push(stack, "int", &i1, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_push(stack, "int", &i2, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_pop_arg(stack, "int", NULL));
    test_int_success(parser_stack_frame_pop(stack));

    sum = 0;
    test_int_success(parser_stack_pop(stack, "int", NULL));
    test_uint_eq(parser_stack_size(stack), 0);
    test_int_eq(sum, i1);
}

TEST_CASE_ABORT(parser_stack_pop_p_invalid_magic)
{
    parser_stack_pop_p((parser_stack_ct)&not_a_stack, "type");
}

TEST_CASE_FIX_ABORT(parser_stack_pop_p_invalid_type, parser_stack_new, no_teardown)
{
    parser_stack_pop_p(stack, NULL);
}

TEST_CASE_FIX(parser_stack_pop_p_empty, parser_stack_new, parser_stack_free)
{
    test_ptr_error(parser_stack_pop_p(stack, "type"), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_pop_p, parser_stack_new, parser_stack_free)
{
    const char *foo;

    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_ptr_success(foo = parser_stack_pop_p(stack, "type"));
    test_uint_eq(parser_stack_size(stack), 0);
    test_str_eq(foo, "foo");
}

TEST_CASE_FIX(parser_stack_pop_p_null, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", NULL, NULL));
    test_ptr_error(parser_stack_pop_p(stack, "type"), E_PARSER_STACK_NULL);
    test_uint_eq(parser_stack_size(stack), 0);
}

TEST_CASE_ABORT(parser_stack_at_invalid_magic)
{
    parser_stack_at((parser_stack_ct)&not_a_stack, "type", 0);
}

TEST_CASE_FIX_ABORT(parser_stack_at_invalid_type, parser_stack_new, no_teardown)
{
    parser_stack_at(stack, NULL, 0);
}

TEST_CASE_FIX(parser_stack_at_empty, parser_stack_new, parser_stack_free)
{
    test_ptr_error(parser_stack_at(stack, "type", 0), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_at_oob, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_ptr_error(parser_stack_at(stack, "type", 1), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_at, parser_stack_new, parser_stack_free)
{
    const char **foo;

    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_ptr_success(foo = parser_stack_at(stack, "type", 0));
    test_uint_eq(parser_stack_size(stack), 1);
    test_str_eq(*foo, "foo");
}

TEST_CASE_FIX(parser_stack_at_null, parser_stack_new, parser_stack_free)
{
    const void **foo;

    test_int_success(parser_stack_push_p(stack, "type", NULL, NULL));
    test_ptr_success(foo = parser_stack_at(stack, "type", 0));
    test_uint_eq(parser_stack_size(stack), 1);
    test_ptr_eq(*foo, NULL);
}

TEST_CASE_ABORT(parser_stack_get_type_invalid_magic)
{
    parser_stack_get_type((parser_stack_ct)&not_a_stack, 0);
}

TEST_CASE_FIX(parser_stack_get_type_empty, parser_stack_new, parser_stack_free)
{
    test_ptr_error(parser_stack_get_type(stack, 0), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_get_type_oob, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_ptr_error(parser_stack_get_type(stack, 1), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_get_type, parser_stack_new, parser_stack_free)
{
    const char *type;

    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_ptr_success(type = parser_stack_get_type(stack, 0));
    test_str_eq(type, "type");
}

TEST_CASE_ABORT(parser_stack_get_size_invalid_magic)
{
    parser_stack_get_size((parser_stack_ct)&not_a_stack, 0);
}

TEST_CASE_FIX(parser_stack_get_size_empty, parser_stack_new, parser_stack_free)
{
    test_int_error(parser_stack_get_size(stack, 0), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_get_size_oob, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_error(parser_stack_get_size(stack, 1), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_get_size, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_rc_success(parser_stack_get_size(stack, 0), sizeof(char *), -1);
}

TEST_CASE_ABORT(parser_stack_activate_invalid_magic)
{
    parser_stack_activate((parser_stack_ct)&not_a_stack, false);
}

TEST_CASE_ABORT(parser_stack_is_active_invalid_magic)
{
    parser_stack_is_active((parser_stack_ct)&not_a_stack);
}

TEST_CASE_FIX(parser_stack_activate, parser_stack_new, parser_stack_free)
{
    test_void(parser_stack_activate(stack, false));
    test_false(parser_stack_is_active(stack));
    test_void(parser_stack_activate(stack, true));
    test_true(parser_stack_is_active(stack));
}

TEST_CASE_ABORT(parser_stack_size_invalid_magic)
{
    parser_stack_size((parser_stack_ct)&not_a_stack);
}

TEST_CASE_FIX(parser_stack_size, parser_stack_new, parser_stack_free)
{
    test_uint_eq(parser_stack_size(stack), 0);
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_uint_eq(parser_stack_size(stack), 1);
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_uint_eq(parser_stack_size(stack), 2);
    test_int_success(parser_stack_pop(stack, "type", NULL));
    test_uint_eq(parser_stack_size(stack), 1);
    test_int_success(parser_stack_pop(stack, "type", NULL));
    test_uint_eq(parser_stack_size(stack), 0);
}

TEST_CASE_ABORT(parser_stack_drop_invalid_magic)
{
    parser_stack_drop((parser_stack_ct)&not_a_stack, 1);
}

TEST_CASE_FIX(parser_stack_drop_empty, parser_stack_new, parser_stack_free)
{
    test_int_error(parser_stack_drop(stack, 1), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_drop_oob, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_error(parser_stack_drop(stack, 2), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_drop, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_drop(stack, 1));
    test_uint_eq(parser_stack_size(stack), 0);
}

TEST_CASE_FIX(parser_stack_drop_dtor, parser_stack_new, parser_stack_free)
{
    int i = 123;

    sum = 0;
    test_int_success(parser_stack_push(stack, "int", &i, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_drop(stack, 1));
    test_uint_eq(parser_stack_size(stack), 0);
    test_int_eq(sum, i);
}

TEST_CASE_FIX(parser_stack_drop_more, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_push_p(stack, "type", "bar", NULL));
    test_int_success(parser_stack_push_p(stack, "type", "baz", NULL));
    test_int_success(parser_stack_drop(stack, 2));
    test_uint_eq(parser_stack_size(stack), 1);
}

TEST_CASE_FIX(parser_stack_drop_skip_arg, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_push_p(stack, "type", "bar", NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push_p(stack, "type", "baz", NULL));
    test_int_success(parser_stack_pop_arg(stack, "type", NULL));
    test_int_success(parser_stack_frame_pop(stack));
    test_int_success(parser_stack_drop(stack, 2));
    test_uint_eq(parser_stack_size(stack), 0);
}

TEST_CASE_FIX(parser_stack_drop_dtor_skip_arg, parser_stack_new, parser_stack_free)
{
    int i1 = 1, i2 = 123, i3 = 2;

    test_int_success(parser_stack_push(stack, "int", &i1, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_push(stack, "int", &i2, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push(stack, "int", &i3, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_pop_arg(stack, "int", NULL));
    test_int_success(parser_stack_frame_pop(stack));

    sum = 0;
    test_int_success(parser_stack_drop(stack, 2));
    test_uint_eq(parser_stack_size(stack), 0);
    test_int_eq(sum, i1 + i3);
}

TEST_CASE_ABORT(parser_stack_clear_invalid_magic)
{
    parser_stack_clear((parser_stack_ct)&not_a_stack);
}

TEST_CASE_FIX(parser_stack_clear_empty, parser_stack_new, parser_stack_free)
{
    test_void(parser_stack_clear(stack));
}

TEST_CASE_FIX(parser_stack_clear, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_void(parser_stack_clear(stack));
    test_uint_eq(parser_stack_size(stack), 0);
}

TEST_CASE_FIX(parser_stack_clear_dtor, parser_stack_new, parser_stack_free)
{
    int i = 123;

    sum = 0;
    test_int_success(parser_stack_push(stack, "int", &i, sizeof(int), test_parser_stack_dtor));
    test_void(parser_stack_clear(stack));
    test_uint_eq(parser_stack_size(stack), 0);
    test_int_eq(sum, i);
}

TEST_CASE_FIX(parser_stack_clear_skip_arg, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_push_p(stack, "type", "bar", NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push_p(stack, "type", "baz", NULL));
    test_int_success(parser_stack_pop_arg(stack, "type", NULL));
    test_int_success(parser_stack_frame_pop(stack));
    test_void(parser_stack_clear(stack));
    test_uint_eq(parser_stack_size(stack), 0);
}

TEST_CASE_FIX(parser_stack_clear_dtor_skip_arg, parser_stack_new, parser_stack_free)
{
    int i1 = 1, i2 = 123, i3 = 2;

    test_int_success(parser_stack_push(stack, "int", &i1, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_push(stack, "int", &i2, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push(stack, "int", &i3, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_pop_arg(stack, "int", NULL));
    test_int_success(parser_stack_frame_pop(stack));

    sum = 0;
    test_void(parser_stack_clear(stack));
    test_uint_eq(parser_stack_size(stack), 0);
    test_int_eq(sum, i1 + i3);
}

TEST_CASE_ABORT(parser_stack_frame_depth_invalid_magic)
{
    parser_stack_frame_depth((parser_stack_ct)&not_a_stack);
}

TEST_CASE_ABORT(parser_stack_frame_push_invalid_magic)
{
    parser_stack_frame_push((parser_stack_ct)&not_a_stack);
}

TEST_CASE_FIX(parser_stack_frame_push, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_uint_eq(parser_stack_frame_depth(stack), 1);
    test_uint_eq(parser_stack_size(stack), 0);
}

TEST_CASE_ABORT(parser_stack_frame_pop_invalid_magic)
{
    parser_stack_frame_pop((parser_stack_ct)&not_a_stack);
}

TEST_CASE_FIX(parser_stack_frame_pop_none, parser_stack_new, parser_stack_free)
{
    test_int_error(parser_stack_frame_pop(stack), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_frame_pop, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_frame_pop(stack));
    test_uint_eq(parser_stack_frame_depth(stack), 0);
    test_uint_eq(parser_stack_size(stack), 2);
}

TEST_CASE_ABORT(parser_stack_args_invalid_magic)
{
    parser_stack_args((parser_stack_ct)&not_a_stack);
}

TEST_CASE_FIX(parser_stack_args_no_frame, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_error(parser_stack_args(stack), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_args, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_success(parser_stack_push_p(stack, "type", "bar", NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push_p(stack, "type", "baz", NULL));
    test_rc_success(parser_stack_args(stack), 2, -1);
}

TEST_CASE_ABORT(parser_stack_pop_arg_invalid_magic)
{
    parser_stack_pop_arg((parser_stack_ct)&not_a_stack, "type", NULL);
}

TEST_CASE_FIX_ABORT(parser_stack_pop_arg_invalid_type, parser_stack_new, no_teardown)
{
    parser_stack_pop_arg(stack, NULL, NULL);
}

TEST_CASE_FIX(parser_stack_pop_arg_no_frame, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_error(parser_stack_pop_arg(stack, "type", NULL), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_pop_arg_empty, parser_stack_new, parser_stack_free)
{
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push_p(stack, "type", "foo", NULL));
    test_int_error(parser_stack_pop_arg(stack, "type", NULL), E_PARSER_STACK_MISSING);
}

TEST_CASE_FIX(parser_stack_pop_arg, parser_stack_new, parser_stack_free)
{
    char foo[4];

    test_int_success(parser_stack_push(stack, "type", "foo", 4, NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_pop_arg(stack, "type", foo));
    test_uint_eq(parser_stack_size(stack), 0);
    test_rc_success(parser_stack_args(stack), 0, -1);
    test_str_eq(foo, "foo");
}

TEST_CASE_FIX(parser_stack_pop_arg_more, parser_stack_new, parser_stack_free)
{
    char foo[4];

    test_int_success(parser_stack_push(stack, "type", "foo", 4, NULL));
    test_int_success(parser_stack_push(stack, "type", "bar", 4, NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_pop_arg(stack, "type", foo));
    test_rc_success(parser_stack_args(stack), 1, -1);
    test_str_eq(foo, "bar");
    test_int_success(parser_stack_pop_arg(stack, "type", foo));
    test_rc_success(parser_stack_args(stack), 0, -1);
    test_str_eq(foo, "foo");
}

TEST_CASE_FIX(parser_stack_pop_arg_buried, parser_stack_new, parser_stack_free)
{
    char foo[4];

    test_int_success(parser_stack_push(stack, "type", "foo", 4, NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push(stack, "type", "bar", 4, NULL));
    test_int_success(parser_stack_pop_arg(stack, "type", foo));
    test_uint_eq(parser_stack_size(stack), 1);
    test_rc_success(parser_stack_args(stack), 0, -1);
    test_str_eq(foo, "foo");
}

TEST_CASE_FIX(parser_stack_pop_arg_buried_more, parser_stack_new, parser_stack_free)
{
    char foo[4];

    test_int_success(parser_stack_push(stack, "type", "foo", 4, NULL));
    test_int_success(parser_stack_push(stack, "type", "bar", 4, NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_push(stack, "type", "baz", 4, NULL));
    test_int_success(parser_stack_pop_arg(stack, "type", foo));
    test_rc_success(parser_stack_args(stack), 1, -1);
    test_str_eq(foo, "bar");
    test_int_success(parser_stack_pop_arg(stack, "type", foo));
    test_rc_success(parser_stack_args(stack), 0, -1);
    test_str_eq(foo, "foo");
}

TEST_CASE_FIX(parser_stack_pop_arg_null, parser_stack_new, parser_stack_free)
{
    char foo[4] = "bar";

    test_int_success(parser_stack_push(stack, "type", NULL, 0, NULL));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_pop_arg(stack, "type", foo));
    test_rc_success(parser_stack_args(stack), 0, -1);
    test_str_eq(foo, "bar");
}

TEST_CASE_FIX(parser_stack_pop_arg_dtor, parser_stack_new, parser_stack_free)
{
    int i = 123;

    sum = 0;
    test_int_success(parser_stack_push(stack, "int", &i, sizeof(int), test_parser_stack_dtor));
    test_int_success(parser_stack_frame_push(stack));
    test_int_success(parser_stack_pop_arg(stack, "int", NULL));
    test_rc_success(parser_stack_args(stack), 0, -1);
    test_int_eq(sum, i);
}

int test_suite_parsers_stack(void *param)
{
    return error_pass_int(test_run_cases("stack",
        test_case(parser_stack_free_invalid_magic),
        test_case(parser_stack_new_free),
        test_case(parser_stack_free_dtor),
        test_case(parser_stack_free_skip_arg),
        test_case(parser_stack_free_dtor_skip_arg),

        test_case(parser_stack_push_invalid_magic),
        test_case(parser_stack_push_invalid_type),
        test_case(parser_stack_push_invalid_data),
        test_case(parser_stack_push),
        test_case(parser_stack_push_null),
        test_case(parser_stack_push_p_invalid_magic),
        test_case(parser_stack_push_p_invalid_type),
        test_case(parser_stack_push_p),
        test_case(parser_stack_push_p_null),

        test_case(parser_stack_pop_invalid_magic),
        test_case(parser_stack_pop_invalid_type),
        test_case(parser_stack_pop_empty),
        test_case(parser_stack_pop),
        test_case(parser_stack_pop_null),
        test_case(parser_stack_pop_dtor),
        test_case(parser_stack_pop_skip_arg),
        test_case(parser_stack_pop_dtor_skip_arg),
        test_case(parser_stack_pop_p_invalid_magic),
        test_case(parser_stack_pop_p_invalid_type),
        test_case(parser_stack_pop_p_empty),
        test_case(parser_stack_pop_p),
        test_case(parser_stack_pop_p_null),

        test_case(parser_stack_at_invalid_magic),
        test_case(parser_stack_at_invalid_type),
        test_case(parser_stack_at_empty),
        test_case(parser_stack_at_oob),
        test_case(parser_stack_at),
        test_case(parser_stack_at_null),

        test_case(parser_stack_get_type_invalid_magic),
        test_case(parser_stack_get_type_empty),
        test_case(parser_stack_get_type_oob),
        test_case(parser_stack_get_type),

        test_case(parser_stack_get_size_invalid_magic),
        test_case(parser_stack_get_size_empty),
        test_case(parser_stack_get_size_oob),
        test_case(parser_stack_get_size),

        test_case(parser_stack_activate_invalid_magic),
        test_case(parser_stack_is_active_invalid_magic),
        test_case(parser_stack_activate),

        test_case(parser_stack_size_invalid_magic),
        test_case(parser_stack_size),

        test_case(parser_stack_drop_invalid_magic),
        test_case(parser_stack_drop_empty),
        test_case(parser_stack_drop_oob),
        test_case(parser_stack_drop),
        test_case(parser_stack_drop_dtor),
        test_case(parser_stack_drop_more),
        test_case(parser_stack_drop_skip_arg),
        test_case(parser_stack_drop_dtor_skip_arg),

        test_case(parser_stack_clear_invalid_magic),
        test_case(parser_stack_clear_empty),
        test_case(parser_stack_clear),
        test_case(parser_stack_clear_dtor),
        test_case(parser_stack_clear_skip_arg),
        test_case(parser_stack_clear_dtor_skip_arg),

        test_case(parser_stack_frame_depth_invalid_magic),
        test_case(parser_stack_frame_push_invalid_magic),
        test_case(parser_stack_frame_push),
        test_case(parser_stack_frame_pop_invalid_magic),
        test_case(parser_stack_frame_pop_none),
        test_case(parser_stack_frame_pop),

        test_case(parser_stack_args_invalid_magic),
        test_case(parser_stack_args_no_frame),
        test_case(parser_stack_args),

        test_case(parser_stack_pop_arg_invalid_magic),
        test_case(parser_stack_pop_arg_invalid_type),
        test_case(parser_stack_pop_arg_no_frame),
        test_case(parser_stack_pop_arg_empty),
        test_case(parser_stack_pop_arg),
        test_case(parser_stack_pop_arg_more),
        test_case(parser_stack_pop_arg_buried),
        test_case(parser_stack_pop_arg_buried_more),
        test_case(parser_stack_pop_arg_null),
        test_case(parser_stack_pop_arg_dtor),

        NULL
    ));
}
