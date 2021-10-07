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

#include <ytil/parser/combinator.h>
#include <ytil/def.h>
#include <stdlib.h>
#include <string.h>

/// default error type for combinator parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


/// Free parser context.
///
/// \implements parser_dtor_cb
static void parser_free_parser(void *ctx)
{
    parser_free(ctx);
}

/// Create new parser with sub parser context.
///
/// \param parse    parse callback
/// \param sub      sub parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
static parser_ct parser_new_parser(parser_parse_cb parse, parser_ct sub)
{
    parser_ct p;

    if(!sub)
        return error_pass(), NULL;

    if(!(p = parser_new(parse, sub, parser_free_parser)))
        return error_pass(), parser_free(sub), NULL;

    return p;
}

/// Free parser list context.
///
/// \implements parser_dtor_cb
static void parser_free_list(void *ctx)
{
    parser_ct *list;

    for(list = ctx; *list; list++)
        parser_free(*list);

    free(ctx);
}

/// Create new parser with sub parser list context.
///
/// \param parse    parse callback
/// \param n        number of sub parsers
/// \param parsers  variadic list of parser_ct
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
static parser_ct parser_new_list(parser_parse_cb parse, size_t n, va_list parsers)
{
    parser_ct p, *list;
    va_list ap;
    size_t i;

    va_copy(ap, parsers);

    for(i = 0; i < n; i++)
    {
        if(!va_arg(ap, parser_ct))
        {
            error_pass();
            goto error;
        }
    }

    va_end(ap);

    if(!(list = calloc(n + 1, sizeof(parser_ct))))
    {
        error_wrap_last_errno(calloc);
        goto error;
    }

    for(i = 0; i < n; i++)
        list[i] = va_arg(parsers, parser_ct);

    if(!(p = parser_new(parse, list, parser_free_list)))
        return error_pass(), parser_free_list(list), NULL;

    return p;

error:
    for(i = 0; i < n; i++)
    {
        if((p = va_arg(parsers, parser_ct)))
            parser_free(p);
    }

    return NULL;
}

/// parser lift value parser state
typedef struct parser_parser_lift_value
{
    parser_ct       p;      ///< parser
    const char      *type;  ///< value type
    size_t          size;   ///< value size
    parser_dtor_cb  dtor;   ///< value destructor
    char            data[]; ///< value data
} parser_parser_lift_value_st;

/// Free parser lift value context.
///
/// \implements parser_dtor_cb
static void parser_free_parser_lift_value(void *ctx)
{
    parser_parser_lift_value_st *lift = ctx;

    parser_free(lift->p);

    if(lift->dtor)
        lift->dtor(lift->data);

    free(lift);
}

/// Create new parser with sub parser lift value context.
///
/// \param parse    parse callback
/// \param sub      sub parser
/// \param type     lift value type
/// \param data     pointer to lift value data
/// \param size     lift value size
/// \param dtor     lift value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
static parser_ct parser_new_parser_lift(parser_parse_cb parse, parser_ct sub, const char *type, const void *data, size_t size, parser_dtor_cb dtor)
{
    parser_parser_lift_value_st *lift;
    parser_ct p;

    assert(type);
    assert(data || !size);

    if(!sub)
        return error_pass(), NULL;

    if(!(lift = calloc(1, sizeof(parser_parser_lift_value_st) + size)))
        return error_wrap_last_errno(calloc), NULL;

    lift->p     = sub;
    lift->type  = type;
    lift->size  = size;
    lift->dtor  = dtor;

    memcpy(lift->data, data, size);

    if(!(p = parser_new(parse, lift, parser_free_parser_lift_value)))
        return error_pass(), parser_free(sub), free(lift), NULL;

    return p;
}

/// parser lift CTOR parser state
typedef struct parser_parser_lift_ctor
{
    parser_ct       p;      ///< parser
    parser_lift_cb  lift;   ///< lift callback
    void            *ctx;   ///< lift callback context
    parser_dtor_cb  dtor;   ///< context destructor
} parser_parser_lift_ctor_st;

/// Free parser lift CTOR context.
///
/// \implements parser_dtor_cb
static void parser_free_parser_lift_ctor(void *ctx)
{
    parser_parser_lift_ctor_st *lift = ctx;

    parser_free(lift->p);

    if(lift->dtor)
        lift->dtor(lift->ctx);

    free(lift);
}

/// Create new parser with sub parser lift CTOR context.
///
/// \param parse    parse callback
/// \param sub      sub parser
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
static parser_ct parser_new_parser_lift_f(parser_parse_cb parse, parser_ct sub, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor)
{
    parser_parser_lift_ctor_st *state;
    parser_ct p;

    assert(lift);

    if(!sub)
        return error_pass(), NULL;

    if(!(state = calloc(1, sizeof(parser_parser_lift_ctor_st))))
        return error_wrap_last_errno(calloc), NULL;

    state->p    = sub;
    state->lift = lift;
    state->ctx  = (void *)ctx;
    state->dtor = dtor;

    if(!(p = parser_new(parse, state, parser_free_parser_lift_ctor)))
        return error_pass(), parser_free(sub), free(state), NULL;

    return p;
}

/// Parse callback for parser asserting success of a parser.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_assert(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct p = ctx;

    if(parser_parse(p, input, size, NULL) < 0)
        return error_pass(), -1;

    return 0;
}

parser_ct parser_assert(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_assert, p));
}

/// Parse callback for parser inverting the status of a parser.
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

/// Parse callback for parser inverting the status of a parser
/// and lifting a value if it fails.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_not_lift_value(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_parser_lift_value_st *lift = ctx;

    if(parser_parse(lift->p, input, size, NULL) >= 0)
        return error_set(E_PARSER_FAIL), -1;

    if(!error_check(0, 1, E_PARSER_FAIL))
        return error_pass(), -1;

    if(stack && parser_stack_push(stack, lift->type, lift->data, lift->size, lift->dtor))
        return error_pass(), -1;

    return 0;
}

parser_ct parser_not_lift(const char *type, const void *data, size_t size, parser_dtor_cb dtor, parser_ct p)
{
    return error_pass_ptr(parser_new_parser_lift(
        parser_parse_not_lift_value, p, type, data, size, dtor));
}

parser_ct parser_not_lift_p(const char *type, const void *ptr, parser_dtor_cb dtor, parser_ct p)
{
    return error_pass_ptr(parser_not_lift(type, &ptr, sizeof(void *), dtor, p));
}

/// Parse callback for parser inverting the status of a parser
/// and lifting values via a CTOR if it fails.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_not_lift_ctor(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_parser_lift_ctor_st *lift = ctx;

    if(parser_parse(lift->p, input, size, NULL) >= 0)
        return error_set(E_PARSER_FAIL), -1;

    if(!error_check(0, 1, E_PARSER_FAIL))
        return error_pass(), -1;

    if(stack && lift->lift(stack, lift->ctx))
        return error_pass(), -1;

    return 0;
}

parser_ct parser_not_lift_f(parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor, parser_ct p)
{
    return error_pass_ptr(parser_new_parser_lift_f(
        parser_parse_not_lift_ctor, p, lift, ctx, dtor));
}

/// Parse callback for parser trying to execute a parser.
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

/// Parse callback for parser trying to execute a parser
/// and lifting a value if it fails.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_maybe_lift_value(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_parser_lift_value_st *lift = ctx;
    ssize_t count;

    if((count = parser_parse(lift->p, input, size, stack)) >= 0)
    {
        assert((size_t)count <= size);

        return count;
    }

    if(!error_check(0, 1, E_PARSER_FAIL))
        return error_pass(), -1;

    if(stack && parser_stack_push(stack, lift->type, lift->data, lift->size, lift->dtor))
        return error_pass(), -1;

    return 0;
}

parser_ct parser_maybe_lift(const char *type, const void *data, size_t size, parser_dtor_cb dtor, parser_ct p)
{
    return error_pass_ptr(parser_new_parser_lift(
        parser_parse_maybe_lift_value, p, type, data, size, dtor));
}

parser_ct parser_maybe_lift_p(const char *type, const void *ptr, parser_dtor_cb dtor, parser_ct p)
{
    return error_pass_ptr(parser_maybe_lift(type, &ptr, sizeof(void *), dtor, p));
}

/// Parse callback for parser trying to execute a parser
/// and lifting values via a CTOR if it fails.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_maybe_lift_ctor(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_parser_lift_ctor_st *lift = ctx;
    ssize_t count;

    if((count = parser_parse(lift->p, input, size, stack)) >= 0)
    {
        assert((size_t)count <= size);

        return count;
    }

    if(!error_check(0, 1, E_PARSER_FAIL))
        return error_pass(), -1;

    if(stack && lift->lift(stack, lift->ctx))
        return error_pass(), -1;

    return 0;
}

parser_ct parser_maybe_lift_f(parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor, parser_ct p)
{
    return error_pass_ptr(parser_new_parser_lift_f(
        parser_parse_maybe_lift_ctor, p, lift, ctx, dtor));
}

/// Parse callback for parser trying to execute a parser as many times as possible.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_many(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct p = ctx;
    const char *ptr;
    ssize_t count;

    for(ptr = input;; ptr += count, size -= count)
    {
        if((count = parser_parse(p, ptr, size, stack)) >= 0)
        {
            assert((size_t)count <= size);

            continue;
        }

        if(!error_check(0, 1, E_PARSER_FAIL))
            return error_pass(), -1;

        return ptr - (const char *)input;
    }
}

parser_ct parser_many(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_many, p));
}

/// repeat/min/max parser state
typedef struct parser_repeat
{
    parser_ct   p;  ///< sub parser
    size_t      n;  ///< number of exact/min/max executions
} parser_repeat_st;

/// Free repeat context.
///
/// \implements parser_dtor_cb
static void parser_free_repeat(void *ctx)
{
    parser_repeat_st *repeat = ctx;

    parser_free(repeat->p);
    free(repeat);
}

/// Parse callback for parser executing a parser an exact number of times.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_repeat(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_repeat_st *repeat = ctx;
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
    parser_repeat_st *repeat;

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
        if(!(repeat = calloc(1, sizeof(parser_repeat_st))))
            return error_wrap_last_errno(calloc), parser_free(p), NULL;

        repeat->p   = p;
        repeat->n   = n;

        if(!(p = parser_new(parser_parse_repeat, repeat, parser_free_repeat)))
            return error_pass(), parser_free(repeat->p), free(repeat), NULL;

        return p;
    }
}

/// Parse callback for parser executing a parser at least once.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_min1(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct p = ctx;
    const char *ptr;
    ssize_t count;
    size_t n;

    for(ptr = input, n = 0;; ptr += count, size -= count, n++)
    {
        if((count = parser_parse(p, ptr, size, stack)) < 0)
        {
            if(!error_check(0, 1, E_PARSER_FAIL))
                return error_pass(), -1;

            if(!n)
                return error_pass(), -1;

            return ptr - (const char *)input;
        }

        assert((size_t)count <= size);
    }
}

parser_ct parser_min1(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_min1, p));
}

/// Parse callback for parser executing a parser at least a number of times.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_min(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_repeat_st *min = ctx;
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
    parser_repeat_st *min;

    if(!p)
        return error_pass(), NULL;

    switch(n)
    {
    case 0:
        return error_pass_ptr(parser_many(p));

    case 1:
        return error_pass_ptr(parser_min1(p));

    default:
        if(!(min = calloc(1, sizeof(parser_repeat_st))))
            return error_wrap_last_errno(calloc), parser_free(p), NULL;

        min->p  = p;
        min->n  = n;

        if(!(p = parser_new(parser_parse_min, min, parser_free_repeat)))
            return error_pass(), parser_free(min->p), free(min), NULL;

        return p;
    }
}

/// Parse callback for parser executing a parser at most a number of times.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_max(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_repeat_st *max = ctx;
    const char *ptr;
    ssize_t count;
    size_t n;

    for(ptr = input, n = 0; n < max->n; ptr += count, size -= count, n++)
    {
        if((count = parser_parse(max->p, ptr, size, stack)) >= 0)
        {
            assert((size_t)count <= size);

            continue;
        }

        if(!error_check(0, 1, E_PARSER_FAIL))
            return error_pass(), -1;

        break;
    }

    return ptr - (const char *)input;
}

parser_ct parser_max(size_t n, parser_ct p)
{
    parser_repeat_st *max;

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
        if(!(max = calloc(1, sizeof(parser_repeat_st))))
            return error_wrap_last_errno(calloc), parser_free(p), NULL;

        max->p  = p;
        max->n  = n;

        if(!(p = parser_new(parser_parse_max, max, parser_free_repeat)))
            return error_pass(), parser_free(max->p), free(max), NULL;

        return p;
    }
}

/// minmax parser state
typedef struct parser_minmax
{
    parser_ct   p;      ///< sub parser
    size_t      min;    ///< min number of executions
    size_t      max;    ///< max number of executions
} parser_minmax_st;

/// Free minmax context.
///
/// \implements parser_dtor_cb
static void parser_free_minmax(void *ctx)
{
    parser_minmax_st *minmax = ctx;

    parser_free(minmax->p);
    free(minmax);
}

/// Parse callback for parser executing a parser a minimum number of times
/// but at most a maximum number of times.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_minmax(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_minmax_st *minmax = ctx;
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
    parser_minmax_st *minmax;

    assert(min <= max);

    if(!p)
        return error_pass(), NULL;

    if(!min)
        return error_pass_ptr(parser_max(max, p));

    if(min == max)
        return error_pass_ptr(parser_repeat(min, p));

    if(!(minmax = calloc(1, sizeof(parser_minmax_st))))
        return error_wrap_last_errno(calloc), parser_free(p), NULL;

    minmax->p   = p;
    minmax->min = min;
    minmax->max = max;

    if(!(p = parser_new(parser_parse_minmax, minmax, parser_free_minmax)))
        return error_pass(), parser_free(minmax->p), free(minmax), NULL;

    return p;
}

/// Parse callback for parser executing a number of parsers in sequence.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_seq(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct *list;
    const char *ptr;
    ssize_t count;

    for(list = ctx, ptr = input; *list; list++, ptr += count, size -= count)
    {
        if((count = parser_parse(*list, ptr, size, stack)) < 0)
            return error_pass(), -1;

        assert((size_t)count <= size);
    }

    return ptr - (const char*)input;
}

parser_ct parser_seq(size_t n, ...)
{
    va_list parsers;
    parser_ct p;

    va_start(parsers, n);
    p = parser_seq_v(n, parsers);
    va_end(parsers);

    return error_pass_ptr(p);
}

parser_ct parser_seq_v(size_t n, va_list parsers)
{
    parser_ct p;

    switch(n)
    {
    case 0:
        p = parser_success();
        break;

    case 1:
        p = va_arg(parsers, parser_ct);
        break;

    default:
        p = parser_new_list(parser_parse_seq, n, parsers);
    }

    return error_pass_ptr(p);
}

/// Parse callback for parser combining a number of parsers with logical AND.
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
    parser_ct p;

    switch(n)
    {
    case 0:
        p = parser_success();
        break;

    case 1:
        p = va_arg(parsers, parser_ct);
        break;

    default:
        p = parser_new_list(parser_parse_and, n, parsers);
    }

    return error_pass_ptr(p);
}

/// Parse callback for parser combining a number of parsers with logical OR.
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
    parser_ct p;

    switch(n)
    {
    case 0:
        p = parser_success();
        break;

    case 1:
        p = va_arg(parsers, parser_ct);
        break;

    default:
        p = parser_new_list(parser_parse_or, n, parsers);
    }

    return error_pass_ptr(p);
}
