/*
 * Copyright (c) 2020 Martin Rödel a.k.a. Yomin Nimoy
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

#include "parser.h"
#include <ytil/test/test.h>
#include <ytil/parser/parser.h>

static const struct not_a_parser
{
    int foo;
} not_a_parser = { 123 };

static parser_ct parser;


static ssize_t _test_parser_parse(const char *input, size_t size, bool result, void *parser_ctx, parser_stack_ct stack, parser_parse_fold_cb parse, void *parse_ctx)
{
    return 0;
}

TEST_SETUP(parser_new)
{
    test_ptr_success(parser = parser_new(_test_parser_parse, NULL, NULL, NULL));
}

TEST_TEARDOWN(parser_free)
{
    parser_free(parser);
}

TEST_CASE_ABORT(parser_new_invalid_parse)
{
    parser_new(NULL, NULL, NULL, NULL);
}

TEST_CASE_ABORT(parser_free_invalid_parser)
{
    parser_free((parser_ct)&not_a_parser);
}

TEST_CASE_FIXTURE(parser_new_free, parser_new, parser_free)
{
}

TEST_CASE_ABORT(parser_set_name_invalid_parser)
{
    parser_set_name((parser_ct)&not_a_parser, LIT("test"));
}

TEST_CASE_ABORT_FIXTURE(parser_set_name_invalid_name1, parser_new, parser_free)
{
    parser_set_name(parser, NULL);
}

TEST_CASE_FIXTURE(parser_set_name_invalid_name2, parser_new, NULL)
{
    test_ptr_error(parser_set_name(parser, LIT("")), E_PARSER_INVALID_NAME);
}

TEST_CASE_FIXTURE(parser_set_name, parser_new, parser_free)
{
    test_ptr_success(parser_set_name(parser, LIT("test")));
}

static int _test_parser_show(str_ct str, parser_show_id mode, void *ctx, parser_show_fold_cb show, void *state)
{
    return 0;
}

TEST_CASE_ABORT(parser_set_show_invalid_parser)
{
    parser_set_show((parser_ct)&not_a_parser, _test_parser_show);
}

TEST_CASE_ABORT_FIXTURE(parser_set_show_invalid_show, parser_new, parser_free)
{
    parser_set_show(parser, NULL);
}

TEST_CASE_FIXTURE(parser_set_show, parser_new, parser_free)
{
    test_ptr_success(parser_set_show(parser, _test_parser_show));
}

TEST_CASE_ABORT(parser_define_invalid_parser)
{
    parser_define(LIT("test"), (parser_ct)&not_a_parser);
}

TEST_CASE_ABORT_FIXTURE(parser_define_invalid_name1, parser_new, parser_free)
{
    parser_define(NULL, parser);
}

TEST_CASE_FIXTURE(parser_define_invalid_name2, parser_new, NULL)
{
    test_ptr_error(parser_define(LIT(""), parser), E_PARSER_INVALID_NAME);
}

TEST_CASE_FIXTURE(parser_define, parser_new, parser_free)
{
    test_ptr_success(parser_define(LIT("test"), parser));
}

TEST_CASE_ABORT_FIXTURE(parser_define_defined, parser_new, parser_free)
{
    parser_define(LIT("foo"), parser_define(LIT("test"), parser));
}

test_suite_ct test_suite_parsers_parser(void)
{
    return test_suite_new_with_cases("parser"
        , test_case_new(parser_new_invalid_parse)
        , test_case_new(parser_free_invalid_parser)
        , test_case_new(parser_new_free)
        
        , test_case_new(parser_set_name_invalid_parser)
        , test_case_new(parser_set_name_invalid_name1)
        , test_case_new(parser_set_name_invalid_name2)
        , test_case_new(parser_set_name)
        , test_case_new(parser_set_show_invalid_parser)
        , test_case_new(parser_set_show_invalid_show)
        , test_case_new(parser_set_show)
        
        , test_case_new(parser_define_invalid_parser)
        , test_case_new(parser_define_invalid_name1)
        , test_case_new(parser_define_invalid_name2)
        , test_case_new(parser_define)
        , test_case_new(parser_define_defined)
    );
}
