/*
 * Copyright (c) 2020-2021 Martin Rödel a.k.a. Yomin Nimoy
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
#include <ytil/parser/parser.h>


static const struct not_a_parser
{
    int foo;
} not_a_parser = { 123 };

static parser_stack_ct stack;
static parser_ct parser;
static bool ctx;


static ssize_t test_parser_parse(const void *input, size_t len, void *ctx, parser_stack_ct stack, void *state)
{
    return 0;
}

static void test_parser_dtor(void *ctx)
{
    bool *freed = ctx;

    *freed = true;
}

TEST_SETUP(parser_new)
{
    test_ptr_success(parser = parser_new(test_parser_parse, &ctx, test_parser_dtor));
}

TEST_TEARDOWN(parser_sink)
{
    test_ptr_eq(parser_sink(parser), NULL);
}

TEST_CASE_ABORT(parser_new_invalid_parse)
{
    parser_new(NULL, NULL, NULL);
}

TEST_CASE_FIX(parser_new_free, parser_new, parser_sink)
{
    test_uint_eq(parser_get_ref_count(parser), 1);
    test_true(parser_is_floating(parser));
}

TEST_CASE_ABORT(parser_get_ref_count_invalid_magic)
{
    parser_get_ref_count((parser_ct)&not_a_parser);
}

TEST_CASE_ABORT(parser_is_floating_invalid_magic)
{
    parser_is_floating((parser_ct)&not_a_parser);
}

TEST_CASE_ABORT(parser_ref_invalid_magic)
{
    parser_ref((parser_ct)&not_a_parser);
}

TEST_CASE_ABORT(parser_unref_invalid_magic)
{
    parser_unref((parser_ct)&not_a_parser);
}

TEST_CASE_ABORT(parser_unref_floating)
{
    parser_unref(parser);
}

TEST_CASE_FIX(parser_ref_unref, parser_new, parser_sink)
{
    test_ptr_eq(parser, parser_ref(parser));
    test_uint_eq(parser_get_ref_count(parser), 2);

    test_ptr_eq(parser, parser_unref(parser));
    test_uint_eq(parser_get_ref_count(parser), 1);
    test_true(parser_is_floating(parser));
}

TEST_CASE_ABORT(parser_sink_invalid_magic)
{
    parser_sink((parser_ct)&not_a_parser);
}

TEST_CASE_FIX(parser_sink, parser_new, no_teardown)
{
    ctx = false;
    test_ptr_eq(NULL, parser_sink(parser));
    test_true(ctx);
}

TEST_CASE_FIX(parser_sink_reffed, parser_new, no_teardown)
{
    test_ptr_eq(parser, parser_ref(parser));
    test_ptr_eq(parser, parser_sink(parser));
    test_uint_eq(parser_get_ref_count(parser), 1);
    test_false(parser_is_floating(parser));
    ctx = false;
    test_ptr_eq(NULL, parser_unref(parser));
    test_true(ctx);
}

TEST_CASE_ABORT(parser_ref_sink_invalid_magic)
{
    parser_ref_sink((parser_ct)&not_a_parser);
}

TEST_CASE_FIX(parser_ref_sink, parser_new, no_teardown)
{
    test_ptr_eq(parser, parser_ref_sink(parser));
    test_uint_eq(parser_get_ref_count(parser), 1);
    test_false(parser_is_floating(parser));
    test_ptr_eq(NULL, parser_unref(parser));
}

TEST_CASE_ABORT(parser_get_ctx_invalid_magic)
{
    parser_get_ctx((parser_ct)&not_a_parser);
}

TEST_CASE_FIX(parser_get_ctx, parser_new, parser_sink)
{
    test_ptr_eq(parser_get_ctx(parser), &ctx);
}

TEST_CASE_ABORT(parser_set_ctx_invalid_magic)
{
    parser_set_ctx((parser_ct)&not_a_parser, parser);
}

TEST_CASE_FIX(parser_set_ctx, parser_new, parser_sink)
{
    test_void(parser_set_ctx(parser, parser));
    test_ptr_eq(parser_get_ctx(parser), parser);
}

TEST_SETUP(parser_stack_new)
{
    test_ptr_success(stack = parser_stack_new());
}

TEST_TEARDOWN(parser_stack_free)
{
    test_void(parser_stack_free(stack));
}

TEST_SETUP(parser_init)
{
    test_setup(parser_stack_new);
    test_setup(parser_new);
}

TEST_PSETUP(parser_init, parser_parse_cb parse)
{
    test_setup(parser_stack_new);
    test_ptr_success(parser = parser_new(parse, "ctx", NULL));
}

TEST_TEARDOWN(parser_finish)
{
    test_teardown(parser_sink);
    test_teardown(parser_stack_free);
}

TEST_CASE_FIX_ABORT(parser_parse_invalid_magic, parser_stack_new, no_teardown)
{
    parser_parse((parser_ct)&not_a_parser, "foo", 3, stack, NULL);
}

TEST_CASE_FIX_ABORT(parser_parse_invalid_input, parser_init, no_teardown)
{
    parser_parse(parser, NULL, 0, stack, NULL);
}

TEST_CASE_FIX_ABORT(parser_parse_invalid_stack, parser_new, no_teardown)
{
    parser_parse(parser, "foo", 3, NULL, NULL);
}

static ssize_t test_parser_parse_too_much(const void *input, size_t len, void *ctx, parser_stack_ct stack, void *state)
{
    return 123;
}

TEST_CASE_PFIX_ABORT(parser_parse_too_much, parser_init, parser_finish, test_parser_parse_too_much)
{
    parser_parse(parser, "foo", 3, stack, NULL);
}

static ssize_t test_parser_parse_fail(const void *input, size_t len, void *ctx, parser_stack_ct stack, void *state)
{
    return error_set_s(PARSER, E_PARSER_FAIL), -1;
}

TEST_CASE_PFIX(parser_parse_fail, parser_init, parser_finish, test_parser_parse_fail)
{
    test_int_error(parser_parse(parser, "foo", 3, stack, NULL), E_PARSER_FAIL);
}

static ssize_t test_parser_parse_success(const void *input, size_t len, void *ctx, parser_stack_ct stack, void *state)
{
    test_str_eq(input, "foo");
    test_uint_eq(len, 3);
    test_str_eq(ctx, "ctx");
    test_str_eq(state, "state");

    return len;
}

TEST_CASE_PFIX(parser_parse_success, parser_init, parser_finish, test_parser_parse_success)
{
    test_rc_success(parser_parse(parser, "foo", 3, stack, "state"), 3, -1);
}

int test_suite_parsers_parser(void *param)
{
    return error_pass_int(test_run_cases("parser",
        test_case(parser_new_invalid_parse),
        test_case(parser_new_free),

        test_case(parser_get_ref_count_invalid_magic),
        test_case(parser_is_floating_invalid_magic),

        test_case(parser_ref_invalid_magic),
        test_case(parser_unref_invalid_magic),
        test_case(parser_unref_floating),
        test_case(parser_ref_unref),

        test_case(parser_sink_invalid_magic),
        test_case(parser_sink),
        test_case(parser_sink_reffed),

        test_case(parser_ref_sink_invalid_magic),
        test_case(parser_ref_sink),

        test_case(parser_get_ctx_invalid_magic),
        test_case(parser_get_ctx),
        test_case(parser_set_ctx_invalid_magic),
        test_case(parser_set_ctx),

        test_case(parser_parse_invalid_magic),
        test_case(parser_parse_invalid_input),
        test_case(parser_parse_invalid_stack),
        test_case(parser_parse_too_much),
        test_case(parser_parse_fail),
        test_case(parser_parse_success),

        NULL
    ));
}
