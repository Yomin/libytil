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
#include <ytil/parser/sub.h>
#include <ytil/def.h>
#include <stdlib.h>
#include <string.h>

/// default error type for null parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


/// Parser context for parser_new_lift().
typedef struct parser_ctx_new_lift
{
    parser_parse_cb     parse;      ///< parse callback
    void                *parse_ctx; ///< \p parse context
    parser_dtor_cb      parse_dtor; ///< \p parse_ctx destructor
    bool                success;    ///< lift on success
    const char          *type;      ///< value type
    size_t              size;       ///< value size
    parser_dtor_cb      data_dtor;  ///< value destructor
    char                data[];     ///< value data
} parser_ctx_new_lift_st;

/// Free parser context of parser_new_lift().
///
/// \implements parser_dtor_cb
static void parser_dtor_new_lift(void *ctx)
{
    parser_ctx_new_lift_st *lift = ctx;

    if(lift->parse_dtor)
        lift->parse_dtor(lift->parse_ctx);

    if(lift->data_dtor)
        lift->data_dtor(lift->data);

    free(lift);
}

/// Parse callback for parser_new_lift().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_new_lift(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_new_lift_st *lift = ctx;

    if(lift->parse(input, size, lift->parse_ctx, stack) < 0)
    {
        if(lift->success)
            return error_pass(), -1;

        if(!error_check(0, 1, E_PARSER_FAIL))
            return error_pass(), -1;
    }
    else
    {
        if(!lift->success)
            return 0;
    }

    if(stack && parser_stack_push(stack, lift->type, lift->data, lift->size, lift->data_dtor))
        return error_pass(), -1;

    return 0;
}

/// Create new parser which lifts a value.
///
/// \note If this function fails, the callback context and the lift value
///       are immediately destroyed.
///
/// \par Equivalent
///     parser_seq(2,
///         parser_new(parse, parse_ctx, parse_dtor),
///         parser_lift(type, data, size, data_dtor))
///
/// \param parse        parse callback
/// \param parse_ctx    \p parse context
/// \param parse_dtor   \p parse_ctx destructor
/// \param success      lift on success
/// \param type         lift value type
/// \param data         pointer to lift value data
/// \param size         lift value size
/// \param data_dtor    lift value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
static parser_ct parser_new_lift(parser_parse_cb parse, const void *parse_ctx, parser_dtor_cb parse_dtor, bool success, const char *type, const void *data, size_t size, parser_dtor_cb data_dtor)
{
    parser_ctx_new_lift_st *lift;

    assert(parse);
    assert(type);
    assert(data || !size);

    if(!(lift = calloc(1, sizeof(parser_ctx_new_lift_st) + size)))
    {
        error_wrap_last_errno(calloc);

        if(parse_dtor)
            parse_dtor((void *)parse_ctx);

        if(data_dtor)
            data_dtor((void *)data);

        return NULL;
    }

    lift->parse         = parse;
    lift->parse_ctx     = (void *)parse_ctx;
    lift->parse_dtor    = parse_dtor;
    lift->success       = success;
    lift->type          = type;
    lift->size          = size;
    lift->data_dtor     = data_dtor;

    memcpy(lift->data, data, size);

    return error_pass_ptr(parser_new(parser_parse_new_lift, lift, parser_dtor_new_lift));
}

parser_ct parser_new_lift_success(parser_parse_cb parse, const void *parse_ctx, parser_dtor_cb parse_dtor, const char *type, const void *data, size_t size, parser_dtor_cb data_dtor)
{
    return error_pass_ptr(parser_new_lift(
        parse, parse_ctx, parse_dtor, true, type, data, size, data_dtor));
}

parser_ct parser_new_lift_fail(parser_parse_cb parse, const void *parse_ctx, parser_dtor_cb parse_dtor, const char *type, const void *data, size_t size, parser_dtor_cb data_dtor)
{
    return error_pass_ptr(parser_new_lift(
        parse, parse_ctx, parse_dtor, false, type, data, size, data_dtor));
}

/// Parser context for parser_new_lift_f().
typedef struct parser_ctx_new_lift_f
{
    parser_parse_cb     parse;      ///< parse callback
    void                *parse_ctx; ///< \p parse context
    parser_dtor_cb      parse_dtor; ///< \p parse_ctx destructor
    bool                success;    ///< lift on success
    parser_lift_cb      lift;       ///< lift callback
    void                *lift_ctx;  ///< \p lift context
    parser_dtor_cb      lift_dtor;  ///< \p lift_ctx destructor
} parser_ctx_new_lift_f_st;

/// Free parser context of parser_new_lift_f().
///
/// \implements parser_dtor_cb
static void parser_dtor_new_lift_f(void *ctx)
{
    parser_ctx_new_lift_f_st *lift = ctx;

    if(lift->parse_dtor)
        lift->parse_dtor(lift->parse_ctx);

    if(lift->lift_dtor)
        lift->lift_dtor(lift->lift_ctx);

    free(lift);
}

/// Parse callback for parser_new_lift_f().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_new_lift_f(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_new_lift_f_st *lift = ctx;

    if(lift->parse(input, size, lift->parse_ctx, stack) < 0)
    {
        if(lift->success)
            return error_pass(), -1;

        if(!error_check(0, 1, E_PARSER_FAIL))
            return error_pass(), -1;
    }
    else
    {
        if(!lift->success)
            return 0;
    }

    if(stack && lift->lift(stack, lift->lift_ctx))
        return error_pass(), -1;

    return 0;
}

/// Create new parser which executes a lift CTOR on success.
///
/// \note If this function fails, the callback contexts are immediately destroyed.
///
/// \par Equivalent
///     parser_seq(2,
///         parser_new(parse, parse_ctx, parse_dtor),
///         parser_lift_f(lift, lift_ctx, lift_dtor))
///
/// \param parse        parse callback
/// \param parse_ctx    \p parse context
/// \param parse_dtor   \p parse_ctx destructor
/// \param success      lift on success
/// \param lift         callback to push values onto the stack
/// \param lift_ctx     \p lift context
/// \param lift_dtor    \p lift_ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
static parser_ct parser_new_lift_f(parser_parse_cb parse, const void *parse_ctx, parser_dtor_cb parse_dtor, bool success, parser_lift_cb lift, const void *lift_ctx, parser_dtor_cb lift_dtor)
{
    parser_ctx_new_lift_f_st *state;

    assert(parse);
    assert(lift);

    if(!(state = calloc(1, sizeof(parser_ctx_new_lift_f_st))))
    {
        error_wrap_last_errno(calloc);

        if(parse_dtor)
            parse_dtor((void *)parse_ctx);

        if(lift_dtor)
            lift_dtor((void *)lift_ctx);

        return NULL;
    }

    state->parse        = parse;
    state->parse_ctx    = (void *)parse_ctx;
    state->parse_dtor   = parse_dtor;
    state->success      = success;
    state->lift         = lift;
    state->lift_ctx     = (void *)lift_ctx;
    state->lift_dtor    = lift_dtor;

    return error_pass_ptr(
        parser_new(parser_parse_new_lift_f, state, parser_dtor_new_lift_f));
}

parser_ct parser_new_lift_success_f(parser_parse_cb parse, const void *parse_ctx, parser_dtor_cb parse_dtor, parser_lift_cb lift, const void *lift_ctx, parser_dtor_cb lift_dtor)
{
    return error_pass_ptr(parser_new_lift_f(
        parse, parse_ctx, parse_dtor, true, lift, lift_ctx, lift_dtor));
}

parser_ct parser_new_lift_fail_f(parser_parse_cb parse, const void *parse_ctx, parser_dtor_cb parse_dtor, parser_lift_cb lift, const void *lift_ctx, parser_dtor_cb lift_dtor)
{
    return error_pass_ptr(parser_new_lift_f(
        parse, parse_ctx, parse_dtor, false, lift, lift_ctx, lift_dtor));
}

/// Parse callback for parser_success().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_success(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    return 0;
}

parser_ct parser_success(void)
{
    return error_pass_ptr(parser_new(parser_parse_success, NULL, NULL));
}

/// Parse callback for parser_fail().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_fail(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    return_error_if_reached(E_PARSER_FAIL, -1);
}

parser_ct parser_fail(void)
{
    return error_pass_ptr(parser_new(parser_parse_fail, NULL, NULL));
}

/// Parse callback for parser_abort().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_abort(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    return_error_if_reached(E_PARSER_ABORT, -1);
}

parser_ct parser_abort(void)
{
    return error_pass_ptr(parser_new(parser_parse_abort, NULL, NULL));
}

/// Parser context of parser_abort_e().
typedef struct parser_ctx_abort_e
{
    const char          *name;  ///< function name
    const error_type_st *type;  ///< error type
    int                 code;   ///< error code
} parser_ctx_abort_e_st;

/// Parse callback for parser_abort_e().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_abort_e(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_abort_e_st *abrt = ctx;

    error_set_f(abrt->name, abrt->type, abrt->code, NULL);
    error_push(E_PARSER_ERROR);

    return -1;
}

parser_ct parser_abort_e(const char *name, const error_type_st *type, int code)
{
    parser_ctx_abort_e_st *abrt;

    assert(type);

    if(!(abrt = calloc(1, sizeof(parser_ctx_abort_e_st))))
        return error_wrap_last_errno(calloc), NULL;

    abrt->name  = name;
    abrt->type  = type;
    abrt->code  = code;

    return error_pass_ptr(parser_new(parser_parse_abort_e, abrt, free));
}

/// Parse callback for parser_assert().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_assert(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct p = ctx;
    ssize_t rc;

    if((rc = parser_parse(p, input, size, stack)) >= 0)
        return rc;

    if(!error_check(0, 1, E_PARSER_FAIL))
        return error_pass(), -1;

    error_push(E_PARSER_ABORT);

    return -1;
}

parser_ct parser_assert(parser_ct p)
{
    return error_pass_ptr(parser_new_parser(parser_parse_assert, p));
}

/// Parser context of parser_assert_e().
typedef struct parser_ctx_assert_e
{
    parser_ct               p;      ///< sub parser
    parser_ctx_abort_e_st   error;  ///< error
} parser_ctx_assert_e_st;

/// Free parser context of parser_assert_e().
///
/// \implements parser_dtor_cb
static void parser_dtor_assert_e(void *ctx)
{
    parser_ctx_assert_e_st *ass = ctx;

    parser_unref(ass->p);
    free(ass);
}

/// Parse callback for parser_assert_e().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_assert_e(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_assert_e_st *ass = ctx;
    ssize_t rc;

    if((rc = parser_parse(ass->p, input, size, stack)) >= 0)
        return rc;

    if(!error_check(0, 1, E_PARSER_FAIL))
        return error_pass(), -1;

    error_push_f(ass->error.name, ass->error.type, ass->error.code, NULL);
    error_push(E_PARSER_ERROR);

    return -1;
}

parser_ct parser_assert_e(parser_ct p, const char *name, const error_type_st *type, int code)
{
    parser_ctx_assert_e_st *ass;

    assert(type);

    if(!p)
        return error_pass(), NULL;

    if(!(ass = calloc(1, sizeof(parser_ctx_assert_e_st))))
        return error_wrap_last_errno(calloc), parser_sink(p), NULL;

    ass->p          = parser_ref_sink(p);
    ass->error.name = name;
    ass->error.type = type;
    ass->error.code = code;

    return error_pass_ptr(parser_new(parser_parse_assert_e, ass, parser_dtor_assert_e));
}

/// Parse callback for parser_end().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_end(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    return_error_if_fail(!size, E_PARSER_FAIL, -1);

    return 0;
}

parser_ct parser_end(void)
{
    return error_pass_ptr(parser_new(parser_parse_end, NULL, NULL));
}

/// Parser context of parser_lift().
typedef struct parser_ctx_lift
{
    const char      *type;  ///< value type
    size_t          size;   ///< value size
    parser_dtor_cb  dtor;   ///< value destructor
    char            data[]; ///< value data
} parser_ctx_lift_st;

/// Free parser context of parser_lift().
///
/// \implements parser_dtor_cb
static void parser_dtor_lift(void *ctx)
{
    parser_ctx_lift_st *lift = ctx;

    if(lift->dtor)
        lift->dtor(lift->data);

    free(lift);
}

/// Parse callback for parser_lift().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_lift(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_lift_st *lift = ctx;

    if(stack && parser_stack_push(stack, lift->type, lift->data, lift->size, lift->dtor))
        return error_pass(), -1;

    return 0;
}

parser_ct parser_lift(const char *type, const void *data, size_t size, parser_dtor_cb dtor)
{
    parser_ctx_lift_st *lift;

    assert(type);
    assert(data || !size);

    if(!(lift = calloc(1, sizeof(parser_ctx_lift_st) + size)))
    {
        error_wrap_last_errno(calloc);

        if(dtor)
            dtor((void *)data);

        return NULL;
    }

    lift->type  = type;
    lift->size  = size;
    lift->dtor  = dtor;

    memcpy(lift->data, data, size);

    return error_pass_ptr(parser_new(parser_parse_lift, lift, parser_dtor_lift));
}

parser_ct parser_lift_p(const char *type, const void *ptr, parser_dtor_cb dtor)
{
    return error_pass_ptr(parser_lift(type, &ptr, sizeof(void *), dtor));
}

/// Parser context of parser_lift_f().
typedef struct parser_ctx_lift_f
{
    parser_lift_cb  lift;   ///< lift callback
    void            *ctx;   ///< \p lift context
    parser_dtor_cb  dtor;   ///< \p ctx destructor
} parser_ctx_lift_f_st;

/// Free parser context of parser_lift_f().
///
/// \implements parser_dtor_cb
static void parser_dtor_lift_f(void *ctx)
{
    parser_ctx_lift_f_st *lift = ctx;

    if(lift->dtor)
        lift->dtor(lift->ctx);

    free(lift);
}

/// Parse callback of parser_lift_f().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_lift_f(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ctx_lift_f_st *lift = ctx;

    if(stack && lift->lift(stack, lift->ctx))
        return error_pass(), -1;

    return 0;
}

parser_ct parser_lift_f(parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor)
{
    parser_ctx_lift_f_st *state;

    assert(lift);

    if(!(state = calloc(1, sizeof(parser_ctx_lift_f_st))))
    {
        error_wrap_last_errno(calloc);

        if(dtor)
            dtor((void *)ctx);

        return NULL;
    }

    state->lift = lift;
    state->ctx  = (void *)ctx;
    state->dtor = dtor;

    return error_pass_ptr(parser_new(parser_parse_lift_f, state, parser_dtor_lift_f));
}

/// Parse callback of parser_new_link().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_new_link(const void *input, size_t size, void *ctx, parser_stack_ct stack)
{
    parser_ct p = ctx;

    return error_pass_int(parser_parse(p, input, size, stack));
}

parser_ct parser_new_link(void)
{
    return error_pass_ptr(parser_ref_sink(parser_new(parser_parse_new_link, NULL, NULL)));
}

parser_ct parser_link(parser_ct link, parser_const_ct p)
{
    assert(!parser_is_floating(link));

    if(!p)
        error_pass();
    else
        parser_set_ctx(link, p);

    // Remove reference introduced by parser_new_link().
    // If link was not referenced by p or p failed, it is freed here.
    parser_unref(link);

    return (parser_ct)p;
}
