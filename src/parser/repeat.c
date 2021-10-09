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

#include <ytil/parser/repeat.h>
#include <ytil/parser/sub.h>
#include <ytil/parser/null.h>
#include <ytil/parser/logic.h>
#include <ytil/def.h>
#include <stdlib.h>

/// default error type for repeat parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


/// Parser context for parser_repeat().
typedef struct parser_ctx_repeat
{
    parser_ct   p;  ///< sub parser
    size_t      n;  ///< number of executions
} parser_ctx_repeat_st;

/// Free parser context of parser_repeat().
///
/// \implements parser_dtor_cb
static void parser_dtor_repeat(void *ctx)
{
    parser_ctx_repeat_st *repeat = ctx;

    parser_free(repeat->p);
    free(repeat);
}

/// Parse callback for parser_repeat().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_repeat(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_repeat_st *repeat = ctx;
    const char *ptr;
    ssize_t count;
    size_t n;

    for(ptr = input, n = 0; n < repeat->n; ptr += count, size -= count, n++)
    {
        if((count = parser_parse(repeat->p, ptr, size, stack)) < 0)
            return error_pass(), -1;

        assert((size_t)count <= size);
    }

    return ptr - (const char *)input;
}

parser_ct parser_repeat(size_t n, parser_ct p)
{
    parser_ctx_repeat_st *repeat;

    if(!p)
        return error_pass(), NULL;

    switch(n)
    {
    case 0:
        parser_free(p);
        return error_pass_ptr(parser_success());

    case 1:
        return p;

    default:
        if(!(repeat = calloc(1, sizeof(parser_ctx_repeat_st))))
            return error_wrap_last_errno(calloc), parser_free(p), NULL;

        repeat->p   = p;
        repeat->n   = n;

        if(!(p = parser_new(parser_parse_repeat, repeat, parser_dtor_repeat)))
            return error_pass(), parser_free(repeat->p), free(repeat), NULL;

        return p;
    }
}

/// Parse callback for parser_repeat_x().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_repeat_x(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_repeat_st repeat = { .p = ctx };

    if(parser_stack_pop_arg(stack, "size_t", &repeat.n))
        return error_pass(), -1;

    return error_pass_int(parser_parse_repeat(input, size, &repeat, stack));
}

parser_ct parser_repeat_x(parser_ct p)
{
    return error_wrap_ptr(parser_new_parser(parser_parse_repeat_x, p));
}

/// Parser context for parser_min().
typedef struct parser_ctx_min
{
    parser_ct   p;  ///< sub parser
    size_t      n;  ///< minimum number of executions
} parser_ctx_min_st;

/// Free parser context of parser_min().
///
/// \implements parser_dtor_cb
static void parser_dtor_min(void *ctx)
{
    parser_ctx_min_st *min = ctx;

    parser_free(min->p);
    free(min);
}

/// Parse callback for parser_min().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_min(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_min_st *min = ctx;
    const char *ptr;
    ssize_t count;
    size_t n;

    for(ptr = input, n = 0;; ptr += count, size -= count, n++)
    {
        if((count = parser_parse(min->p, ptr, size, stack)) < 0)
        {
            if(!error_check(0, 1, E_PARSER_FAIL))
                return error_pass(), -1;

            if(n < min->n)
                return error_pass(), -1;

            return ptr - (const char *)input;
        }

        assert((size_t)count <= size);
    }
}

parser_ct parser_min(size_t n, parser_ct p)
{
    parser_ctx_min_st *min;

    if(!p)
        return error_pass(), NULL;

    switch(n)
    {
    case 0:
        return error_pass_ptr(parser_many(p));

    case 1:
        return error_pass_ptr(parser_min1(p));

    default:
        if(!(min = calloc(1, sizeof(parser_ctx_min_st))))
            return error_wrap_last_errno(calloc), parser_free(p), NULL;

        min->p  = p;
        min->n  = n;

        if(!(p = parser_new(parser_parse_min, min, parser_dtor_min)))
            return error_pass(), parser_free(min->p), free(min), NULL;

        return p;
    }
}

/// Parse callback for parser_min1().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_min1(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_min_st min = { .p = ctx, .n = 1 };

    return error_pass_int(parser_parse_min(input, size, &min, stack));
}

parser_ct parser_min1(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_min1, p));
}

/// Parse callback for parser_many().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_many(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_min_st min = { .p = ctx, .n = 0 };

    return error_pass_int(parser_parse_min(input, size, &min, stack));
}

parser_ct parser_many(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_many, p));
}

/// Parser context for parser_minmax().
typedef struct parser_ctx_minmax
{
    parser_ct   p;      ///< sub parser
    size_t      min;    ///< min number of executions
    size_t      max;    ///< max number of executions
} parser_ctx_minmax_st;

/// Free parser context of parser_minmax().
///
/// \implements parser_dtor_cb
static void parser_dtor_minmax(void *ctx)
{
    parser_ctx_minmax_st *minmax = ctx;

    parser_free(minmax->p);
    free(minmax);
}

/// Parse callback for parser_minmax().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_minmax(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_minmax_st *minmax = ctx;
    const char *ptr;
    ssize_t count;
    size_t n;

    for(ptr = input, n = 0; n < minmax->max; ptr += count, size -= count, n++)
    {
        if((count = parser_parse(minmax->p, ptr, size, stack)) >= 0)
        {
            assert((size_t)count <= size);

            continue;
        }

        if(!error_check(0, 1, E_PARSER_FAIL))
            return error_pass(), -1;

        if(n < minmax->min)
            return error_pass(), -1;

        break;
    }

    return ptr - (const char *)input;
}

parser_ct parser_minmax(size_t min, size_t max, parser_ct p)
{
    parser_ctx_minmax_st *minmax;

    assert(min <= max);

    if(!p)
        return error_pass(), NULL;

    if(!min)
        return error_pass_ptr(parser_max(max, p));

    if(min == max)
        return error_pass_ptr(parser_repeat(min, p));

    if(!(minmax = calloc(1, sizeof(parser_ctx_minmax_st))))
        return error_wrap_last_errno(calloc), parser_free(p), NULL;

    minmax->p   = p;
    minmax->min = min;
    minmax->max = max;

    if(!(p = parser_new(parser_parse_minmax, minmax, parser_dtor_minmax)))
        return error_pass(), parser_free(minmax->p), free(minmax), NULL;

    return p;
}

/// Parser context for parser_max().
typedef struct parser_ctx_max
{
    parser_ct   p;  ///< sub parser
    size_t      n;  ///< maximum number of executions
} parser_ctx_max_st;

/// Free parser context of parser_max().
///
/// \implements parser_dtor_cb
static void parser_dtor_max(void *ctx)
{
    parser_ctx_max_st *max = ctx;

    parser_free(max->p);
    free(max);
}

/// Parse callback for parser_max().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_max(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_max_st *max = ctx;
    parser_ctx_minmax_st minmax = { .p = max->p, .min = 0, .max = max->n };

    return error_pass_int(parser_parse_minmax(input, size, &minmax, stack));
}

parser_ct parser_max(size_t n, parser_ct p)
{
    parser_ctx_max_st *max;

    if(!p)
        return error_pass(), NULL;

    switch(n)
    {
    case 0:
        parser_free(p);
        return error_pass_ptr(parser_success());

    case 1:
        return error_pass_ptr(parser_maybe(p));

    default:
        if(!(max = calloc(1, sizeof(parser_ctx_max_st))))
            return error_wrap_last_errno(calloc), parser_free(p), NULL;

        max->p  = p;
        max->n  = n;

        if(!(p = parser_new(parser_parse_max, max, parser_dtor_max)))
            return error_pass(), parser_free(max->p), free(max), NULL;

        return p;
    }
}
