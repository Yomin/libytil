/*
 * Copyright (c) 2018-2021 Martin Rödel aka Yomin
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

/// \file

#include <ytil/parser/string.h>
#include <ytil/parser/null.h>
#include <ytil/def.h>
#include <string.h>
#include <stdlib.h>

/// default error type for string parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


/// Parse callback for parser_string().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_string(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    const char *str = ctx;
    size_t len      = strlen(str);

    if(len > size || strncmp(input, str, len))
        return error_set(E_PARSER_FAIL), -1;

    if(stack && parser_stack_push_p(stack, "cstring", str, NULL))
        return error_pass(), -1;

    return len;
}

parser_ct parser_string(const char *str)
{
    assert(str);

    if(!str[0])
        return error_pass_ptr(parser_success());

    return error_pass_ptr(parser_new(parser_parse_string, str, NULL));
}

parser_ct parser_string_accept(size_t n, ...)
{
    va_list strings;
    parser_ct p;

    va_start(strings, n);
    p = parser_string_accept_v(n, strings);
    va_end(strings);

    return error_pass_ptr(p);
}

/// Parse callback for parser_string_accept().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_string_accept(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    const char **list = ctx;
    ssize_t rc;

    for(; *list; list++)
    {
        if((rc = parser_parse_string(input, size, (void *)*list, stack)) >= 0)
            return rc;

        if(!error_check(0, 1, E_PARSER_FAIL))
            return error_pass(), -1;
    }

    return_error_if_reached(E_PARSER_FAIL, -1);
}

parser_ct parser_string_accept_v(size_t n, va_list strings)
{
    const char **list;
    size_t i;

    if(!n)
        return error_pass_ptr(parser_fail());

    if(n == 1)
        return error_pass_ptr(parser_string(va_arg(strings, const char*)));

    if(!(list = calloc(n + 1, sizeof(const char *))))
        return error_wrap_last_errno(calloc), NULL;

    for(i = 0; i < n; i++)
    {
        list[i] = va_arg(strings, const char*);
        assert(list[i]);
    }

    return error_pass_ptr(parser_new(parser_parse_string_accept, list, free));
}
