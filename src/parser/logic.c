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

#include <ytil/parser/logic.h>
#include <ytil/parser/sub.h>
#include <ytil/def.h>

/// default error type for logic parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


/// Free parser context with sub parser as context.
///
/// \implements parser_dtor_cb
static void parser_dtor_parser(void *ctx)
{
    parser_unref(ctx);
}

/// Parse callback for parser_check().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_check(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct p = ctx;

    if(parser_parse(p, input, size, NULL) < 0)
        return error_pass(), -1;

    return 0; // discard matched input
}

parser_ct parser_check(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_check, p));
}

parser_ct parser_check_lift(parser_ct p, const char *type, const void *data, size_t size, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_new_lift_success(
        parser_parse_check, parser_ref_sink(p), parser_dtor_parser,
        type, data, size, dtor));
}

parser_ct parser_check_lift_p(parser_ct p, const char *type, const void *ptr, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_check_lift(p, type, &ptr, sizeof(void *), dtor));
}

parser_ct parser_check_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_new_lift_success_f(
        parser_parse_check, parser_ref_sink(p), parser_dtor_parser,
        lift, ctx, dtor));
}

/// Parse callback for parser_drop().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_drop(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct p = ctx;

    return error_pass_int(parser_parse(p, input, size, NULL));
}

parser_ct parser_drop(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_drop, p));
}

parser_ct parser_drop_lift(parser_ct p, const char *type, const void *data, size_t size, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_new_lift_success(
        parser_parse_drop, parser_ref_sink(p), parser_dtor_parser,
        type, data, size, dtor));
}

parser_ct parser_drop_lift_p(parser_ct p, const char *type, const void *ptr, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_drop_lift(p, type, &ptr, sizeof(void *), dtor));
}

parser_ct parser_drop_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_new_lift_success_f(
        parser_parse_drop, parser_ref_sink(p), parser_dtor_parser,
        lift, ctx, dtor));
}

/// Parse callback for parser_not().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_not(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct p = ctx;

    if(parser_parse(p, input, size, NULL) >= 0)
        return error_set(E_PARSER_FAIL), -1;

    if(!error_check(0, 1, E_PARSER_FAIL))
        return error_pass(), -1;

    return 0;
}

parser_ct parser_not(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_not, p));
}

parser_ct parser_not_lift(parser_ct p, const char *type, const void *data, size_t size, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_new_lift_success(
        parser_parse_not, parser_ref_sink(p), parser_dtor_parser,
        type, data, size, dtor));
}

parser_ct parser_not_lift_p(parser_ct p, const char *type, const void *ptr, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_not_lift(p, type, &ptr, sizeof(void *), dtor));
}

parser_ct parser_not_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_new_lift_success_f(
        parser_parse_not, parser_ref_sink(p), parser_dtor_parser,
        lift, ctx, dtor));
}

/// Parse callback for parser_maybe().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_maybe(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct p = ctx;
    ssize_t count;

    if((count = parser_parse(p, input, size, stack)) >= 0)
    {
        assert((size_t)count <= size);

        return count;
    }

    if(!error_check(0, 1, E_PARSER_FAIL))
        return error_pass(), -1;

    return 0;
}

parser_ct parser_maybe(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_maybe, p));
}

/// Parse callback for parser_maybe_drop().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_maybe_drop(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    return error_pass_int(parser_parse_maybe(input, size, ctx, NULL));
}

parser_ct parser_maybe_drop(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_maybe_drop, p));
}

/// Parse callback for parser_maybe_lift().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_maybe_lift(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct p = ctx;

    return error_pass_int(parser_parse(p, input, size, stack));
}

parser_ct parser_maybe_lift(parser_ct p, const char *type, const void *data, size_t size, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_new_lift_fail(
        parser_parse_maybe_lift, parser_ref_sink(p), parser_dtor_parser,
        type, data, size, dtor));
}

parser_ct parser_maybe_lift_p(parser_ct p, const char *type, const void *ptr, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_maybe_lift(p, type, &ptr, sizeof(void *), dtor));
}

parser_ct parser_maybe_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_new_lift_fail_f(
        parser_parse_maybe_lift, parser_ref_sink(p), parser_dtor_parser,
        lift, ctx, dtor));
}

/// Parse callback for parser_and().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_and(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct *list;
    ssize_t count = 0;

    for(list = ctx; *list; list++)
    {
        if((count = parser_parse(*list, input, size, list[1] ? NULL : stack)) < 0)
            return error_pass(), -1;

        assert((size_t)count <= size);
    }

    return count;
}

parser_ct parser_and(size_t n, ...)
{
    va_list parsers;
    parser_ct p;

    va_start(parsers, n);
    p = parser_and_v(n, parsers);
    va_end(parsers);

    return error_pass_ptr(p);
}

parser_ct parser_and_v(size_t n, va_list parsers)
{
    if(!n)
        return error_pass_ptr(parser_success());

    if(n == 1)
        return error_pass_ptr(va_arg(parsers, parser_ct));

    return error_pass_ptr(parser_new_parser_list_v(parser_parse_and, n, parsers));
}

/// Parse callback for parser_or().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_or(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct *list;
    ssize_t count;

    for(list = ctx; *list; list++)
    {
        if((count = parser_parse(*list, input, size, stack)) >= 0)
        {
            assert((size_t)count <= size);

            return count;
        }

        if(!error_check(0, 1, E_PARSER_FAIL))
            return error_pass(), -1;
    }

    return_error_if_reached(E_PARSER_FAIL, -1);
}

parser_ct parser_or(size_t n, ...)
{
    va_list parsers;
    parser_ct p;

    va_start(parsers, n);
    p = parser_or_v(n, parsers);
    va_end(parsers);

    return error_pass_ptr(p);
}

parser_ct parser_or_v(size_t n, va_list parsers)
{
    if(!n)
        return error_pass_ptr(parser_success());

    if(n == 1)
        return error_pass_ptr(va_arg(parsers, parser_ct));

    return error_pass_ptr(parser_new_parser_list_v(parser_parse_or, n, parsers));
}
