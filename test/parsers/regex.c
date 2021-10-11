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
#include <ytil/parser/regex.h>

static parser_ct parser;


TEST_TEARDOWN(parser_sink)
{
    test_ptr_eq(parser_sink(parser), NULL);
}

TEST_CASE_FIX(parser_regex, no_setup, parser_sink)
{
    const char *foo = ")foo";

    test_ptr_success(parser = parser_regex());
    test_rc_success(parser_parse(parser, foo, strlen(foo), (parser_stack_ct)1), strlen(foo), -1);
}

int test_suite_parsers_regex(void *param)
{
    return error_pass_int(test_run_cases("regex",
        test_case(parser_regex),

        NULL
    ));
}
