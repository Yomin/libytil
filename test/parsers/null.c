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
#include <ytil/parser/null.h>
#include <errno.h>

#define ERROR_TYPE_DEFAULT ERROR_TYPE_ERRNO

static parser_stack_ct stack;
static parser_ct parser;


TEST_SETUP(parser_init)
{
    test_ptr_success(stack = parser_stack_new());
}

TEST_TEARDOWN(parser_stack_free)
{
    test_void(parser_stack_free(stack));
}

TEST_TEARDOWN(parser_finish)
{
    test_ptr_eq(parser_sink(parser), NULL);
    test_teardown(parser_stack_free);
}

TEST_CASE_FIX(parser_success, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_success());
    test_rc_success(parser_parse(parser, "foo", 3, stack, NULL), 0, -1);
}

TEST_CASE_FIX(parser_fail, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_fail());
    test_int_error(parser_parse(parser, "foo", 3, stack, NULL), E_PARSER_FAIL);
}

TEST_CASE_FIX(parser_abort, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_abort());
    test_int_error(parser_parse(parser, "foo", 3, stack, NULL), E_PARSER_ABORT);
}

TEST_CASE_FIX(parser_abort_e, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_abort_e("func", EINVAL));
    test_int_error(parser_parse(parser, "foo", 3, stack, NULL), E_PARSER_ERROR);
    test_error(1, ERRNO, EINVAL);
}

TEST_CASE_FIX(parser_abort_es, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_abort_es("func", ERROR_TYPE(ERRNO), ENOENT));
    test_int_error(parser_parse(parser, "foo", 3, stack, NULL), E_PARSER_ERROR);
    test_error(1, ERRNO, ENOENT);
}

static parser_ct parser_enomem(void)
{
    return error_wrap_errno("func", ENOMEM), NULL;
}

TEST_CASE(parser_assert_error)
{
    test_ptr_error(parser_assert(parser_enomem()), E_GENERIC_OOM);
}

TEST_CASE_FIX(parser_assert_fail, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_assert(parser_fail()));
    test_int_error(parser_parse(parser, "foo", 3, stack, NULL), E_PARSER_ABORT);
    test_error(1, PARSER, E_PARSER_FAIL);
}

TEST_CASE_FIX(parser_assert_success, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_assert(parser_success()));
    test_rc_success(parser_parse(parser, "foo", 3, stack, NULL), 0, -1);
}

TEST_CASE(parser_assert_e_error)
{
    test_ptr_error(parser_assert_e(parser_enomem(), "func", EINVAL), E_GENERIC_OOM);
}

TEST_CASE_FIX(parser_assert_e_fail, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_assert_e(parser_fail(), "func", EINVAL));
    test_int_error(parser_parse(parser, "foo", 3, stack, NULL), E_PARSER_ERROR);
    test_error(1, ERRNO, EINVAL);
}

TEST_CASE_FIX(parser_assert_e_success, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_assert_e(parser_success(), "func", EINVAL));
    test_rc_success(parser_parse(parser, "foo", 3, stack, NULL), 0, -1);
}

TEST_CASE(parser_assert_es_error)
{
    test_ptr_error(parser_assert_es(parser_enomem(), "func", ERROR_TYPE(ERRNO), ENOENT), E_GENERIC_OOM);
}

TEST_CASE_FIX(parser_assert_es_fail, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_assert_es(parser_fail(), "func", ERROR_TYPE(ERRNO), ENOENT));
    test_int_error(parser_parse(parser, "foo", 3, stack, NULL), E_PARSER_ERROR);
    test_error(1, ERRNO, ENOENT);
}

TEST_CASE_FIX(parser_assert_es_success, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_assert_es(parser_success(), "func", ERROR_TYPE(ERRNO), ENOENT));
    test_rc_success(parser_parse(parser, "foo", 3, stack, NULL), 0, -1);
}

TEST_CASE_FIX(parser_end_fail, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_end());
    test_int_error(parser_parse(parser, "foo", 3, stack, NULL), E_PARSER_FAIL);
}

TEST_CASE_FIX(parser_end_success, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_end());
    test_rc_success(parser_parse(parser, "", 0, stack, NULL), 0, -1);
}

TEST_CASE_FIX(parser_lift, parser_init, parser_finish)
{
    char foo[4] = "baz";

    test_ptr_success(parser = parser_lift("char[]", "foo", 3));
    test_rc_success(parser_parse(parser, "bar", 3, stack, NULL), 0, -1);
    test_int_success(parser_stack_pop(stack, "char[]", foo));
    test_str_eq(foo, "foo");
}

TEST_CASE_FIX(parser_lift_p, parser_init, parser_finish)
{
    const char *foo = "baz";

    test_ptr_success(parser = parser_lift_p("string", "foo"));
    test_rc_success(parser_parse(parser, "bar", 3, stack, NULL), 0, -1);
    test_ptr_success(foo = parser_stack_pop_p(stack, "string"));
    test_str_eq(foo, "foo");
}

static int parser_test_lift(parser_stack_ct stack, void *ctx, void *state)
{
    test_int_success(parser_stack_push_p(stack, "string", ctx, NULL));
    test_int_success(parser_stack_push_p(stack, "string", state, NULL));

    return 0;
}

TEST_CASE_FIX(parser_lift_f, parser_init, parser_finish)
{
    const char *foo = "baz";

    test_ptr_success(parser = parser_lift_f(parser_test_lift, "foo", NULL));
    test_rc_success(parser_parse(parser, "bar", 3, stack, "state"), 0, -1);
    test_ptr_success(foo = parser_stack_pop_p(stack, "string"));
    test_str_eq(foo, "state");
    test_ptr_success(foo = parser_stack_pop_p(stack, "string"));
    test_str_eq(foo, "foo");
}

TEST_CASE_ABORT(parser_new_lift_success_invalid_parse)
{
    parser_new_lift_success(NULL, NULL, NULL, "char[]", "foo", 3);
}

static ssize_t test_parser_lift_success(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    int *val = ctx;

    test_int_success(parser_stack_push_p(stack, "string", input, NULL));
    test_int_success(parser_stack_push(stack, "int", val, sizeof(int), NULL));
    test_int_success(parser_stack_push_p(stack, "string", state, NULL));

    return size;
}

TEST_CASE_ABORT(parser_new_lift_success_invalid_type)
{
    parser_new_lift_success(test_parser_lift_success, NULL, NULL, NULL, "foo", 3);
}

TEST_CASE_ABORT(parser_new_lift_success_invalid_data)
{
    parser_new_lift_success(test_parser_lift_success, NULL, NULL, "char[]", NULL, 3);
}

static void test_parser_lift_dtor(void *ctx)
{
    int *val = ctx;

    *val = 42;
}

TEST_CASE_FIX(parser_new_lift_success_success, parser_init, parser_stack_free)
{
    char buf[5] = "foo", *str;
    int val1 = 123, val2 = 0;

    test_ptr_success(parser = parser_new_lift_success(test_parser_lift_success, &val1, test_parser_lift_dtor, "char[]", "lift", 4));
    test_rc_success(parser_parse(parser, "input", 3, stack, "state"), 3, -1);
    test_uint_eq(parser_stack_size(stack), 4);
    test_int_success(parser_stack_pop(stack, "char[]", buf));
    test_str_eq(buf, "lift");
    test_ptr_success(str = parser_stack_pop_p(stack, "string"));
    test_str_eq(str, "state");
    test_int_success(parser_stack_pop(stack, "int", &val2));
    test_int_eq(val1, val2);
    test_ptr_success(str = parser_stack_pop_p(stack, "string"));
    test_str_eq(str, "input");
    test_ptr_eq(parser_sink(parser), NULL);
    test_int_eq(val1, 42);
}

static ssize_t test_parser_lift_fail(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    return error_set_s(PARSER, E_PARSER_FAIL), -1;
}

TEST_CASE_FIX(parser_new_lift_success_fail, parser_init, parser_finish)
{
    test_ptr_success(parser = parser_new_lift_success(test_parser_lift_fail, NULL, NULL, "char[]", "lift", 4));
    test_int_error(parser_parse(parser, "input", 3, stack, "state"), E_PARSER_FAIL);
    test_uint_eq(parser_stack_size(stack), 0);
}

int test_suite_parsers_null(void *param)
{
    return error_pass_int(test_run_cases("null",
        test_case(parser_success),
        test_case(parser_fail),

        test_case(parser_abort),
        test_case(parser_abort_e),
        test_case(parser_abort_es),

        test_case(parser_assert_error),
        test_case(parser_assert_fail),
        test_case(parser_assert_success),
        test_case(parser_assert_e_error),
        test_case(parser_assert_e_fail),
        test_case(parser_assert_e_success),
        test_case(parser_assert_es_error),
        test_case(parser_assert_es_fail),
        test_case(parser_assert_es_success),

        test_case(parser_end_fail),
        test_case(parser_end_success),

        test_case(parser_lift),
        test_case(parser_lift_p),
        test_case(parser_lift_f),

        test_case(parser_new_lift_success_invalid_parse),
        test_case(parser_new_lift_success_invalid_type),
        test_case(parser_new_lift_success_invalid_data),
        test_case(parser_new_lift_success_success),
        test_case(parser_new_lift_success_fail),

        NULL
    ));
}
