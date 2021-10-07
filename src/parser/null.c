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

#include <ytil/parser/null.h>
#include <ytil/def.h>
#include <stdlib.h>
#include <string.h>

/// default error type for null parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


/// Parse callback for parser which always succeeds.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_success(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    return 0;
}

parser_ct parser_success(void)
{
    return error_pass_ptr(
        parser_new(parser_parse_success, NULL, NULL));
}

/// Parse callback for parser which always fails.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_fail(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    return_error_if_reached(E_PARSER_FAIL, -1);
}

parser_ct parser_fail(void)
{
    return error_pass_ptr(
        parser_new(parser_parse_fail, NULL, NULL));
}

/// Parse callback for parser matching end of input.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_end(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    return_error_if_fail(!size, E_PARSER_FAIL, -1);

    return 0;
}

parser_ct parser_end(void)
{
    return error_pass_ptr(
        parser_new(parser_parse_end, NULL, NULL));
}

/// lift value parser state
typedef struct parser_lift_value
{
    const char      *type;  ///< value type
    size_t          size;   ///< value size
    parser_dtor_cb  dtor;   ///< value destructor
    char            data[]; ///< value data
} parser_lift_value_st;

/// Free lift value context.
///
/// \implements parser_dtor_cb
static void parser_free_lift_value(void *ctx)
{
    parser_lift_value_st *lift = ctx;

    if(lift->dtor)
        lift->dtor(lift->data);

    free(lift);
}

/// Parse callback for parser lifting a value.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_lift_value(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_lift_value_st *lift = ctx;

    if(stack && parser_stack_push(stack, lift->type, lift->data, lift->size, lift->dtor))
        return error_pass(), -1;

    return 0;
}

parser_ct parser_lift(const char *type, const void *data, size_t size, parser_dtor_cb dtor)
{
    parser_lift_value_st *lift;
    parser_ct p;

    assert(type);
    assert(data || !size);

    if(!(lift = calloc(1, sizeof(parser_lift_value_st) + size)))
        return error_wrap_last_errno(calloc), NULL;

    lift->type  = type;
    lift->size  = size;
    lift->dtor  = dtor;

    memcpy(lift->data, data, size);

    if(!(p = parser_new(parser_parse_lift_value, lift, parser_free_lift_value)))
        return error_pass(), free(lift), NULL;

    return p;
}

parser_ct parser_lift_p(const char *type, const void *ptr, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_lift(type, &ptr, sizeof(void *), dtor));
}

/// lift CTOR parser state
typedef struct parser_lift_ctor
{
    parser_lift_cb  lift;   ///< lift callback
    void            *ctx;   ///< lift callback context
    parser_dtor_cb  dtor;   ///< context destructor
} parser_lift_ctor_st;

/// Free lift CTOR context.
///
/// \implements parser_dtor_cb
static void parser_free_lift_ctor(void *ctx)
{
    parser_lift_ctor_st *lift = ctx;

    if(lift->dtor)
        lift->dtor(lift->ctx);

    free(lift);
}

/// Parse callback for parser lifting values via a CTOR.
///
/// \implements parser_parse_cb
static ssize_t parser_parse_lift_ctor(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_lift_ctor_st *lift = ctx;

    if(stack && lift->lift(stack, lift->ctx))
        return error_pass(), -1;

    return 0;
}

parser_ct parser_lift_f(parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor)
{
    parser_lift_ctor_st *state;
    parser_ct p;

    assert(lift);

    if(!(state = calloc(1, sizeof(parser_lift_ctor_st))))
        return error_wrap_last_errno(calloc), NULL;

    state->lift = lift;
    state->ctx  = (void *)ctx;
    state->dtor = dtor;

    if(!(p = parser_new(parser_parse_lift_ctor, state, parser_free_lift_ctor)))
        return error_pass(), free(state), NULL;

    return p;
}
