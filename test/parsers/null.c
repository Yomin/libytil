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

static parser_ct parser;


TEST_TEARDOWN(parser_sink)
{
    test_ptr_eq(parser_sink(parser), NULL);
}

TEST_CASE_FIX(parser_success, no_setup, parser_sink)
{
    test_ptr_success(parser = parser_success());
    test_rc_success(parser_parse(parser, "foo", 3, NULL), 0, -1);
}

TEST_CASE_FIX(parser_fail, no_setup, parser_sink)
{
    test_ptr_success(parser = parser_fail());
    test_int_error(parser_parse(parser, "foo", 3, NULL), E_PARSER_FAIL);
}

TEST_CASE_FIX(parser_end_fail, no_setup, parser_sink)
{
    test_ptr_success(parser = parser_end());
    test_int_error(parser_parse(parser, "foo", 3, NULL), E_PARSER_FAIL);
}

TEST_CASE_FIX(parser_end_success, no_setup, parser_sink)
{
    test_ptr_success(parser = parser_end());
    test_rc_success(parser_parse(parser, "", 0, NULL), 0, -1);
}

TEST_CASE_FIX(parser_lift, no_setup, parser_sink)
{
    test_ptr_success(parser = parser_lift("char", "foo", 4, NULL));
    test_rc_success(parser_parse(parser, "bar", 3, NULL), 0, -1);
}

TEST_CASE_FIX(parser_lift_p, no_setup, parser_sink)
{
    test_ptr_success(parser = parser_lift_p("string", "foo", NULL));
    test_rc_success(parser_parse(parser, "bar", 3, NULL), 0, -1);
}

static int parser_test_lift(parser_stack_ct stack, void *ctx)
{
    test_int_success(parser_stack_push_p(stack, "string", ctx, NULL));

    return 0;
}

TEST_CASE_FIX(parser_lift_f, no_setup, parser_sink)
{
    test_ptr_success(parser = parser_lift_f(parser_test_lift, "foo", NULL));
    test_rc_success(parser_parse(parser, "bar", 3, NULL), 0, -1);
}

int test_suite_parsers_null(void *param)
{
    return error_pass_int(test_run_cases("null",
        test_case(parser_success),
        test_case(parser_fail),

        test_case(parser_end_fail),
        test_case(parser_end_success),

        test_case(parser_lift),
        test_case(parser_lift_p),
        test_case(parser_lift_f),

        NULL
    ));
}
