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


int parser_stack_push_match(parser_stack_ct stack, const void *data, size_t size)
{
    parser_match_st match = { .data = data, .size = size };

    return error_pass_int(
        parser_stack_push(stack, "match", &match, sizeof(parser_match_st), NULL));
}

/// Parse callback for parser_string().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_string(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char *str = ctx;
    size_t len      = strlen(str);

    if(len > size || strncmp(input, str, len))
        return error_set(E_PARSER_FAIL), -1;

    if(parser_stack_push_match(stack, input, len))
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

/// Parser context for parser_while/until_min().
typedef struct parser_ctx_loop
{
    size_t          n;      ///< minimum number of matches
    ctype_pred_cb   pred;   ///< predicate
    bool            result; ///< expected result
} parser_ctx_loop_st;

/// Parse callback for parser_while/until_min().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_loop(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    parser_ctx_loop_st *loop    = ctx;
    const char *text            = input;
    size_t count;

    for(count = 0; count < size; count++)
    {
        if(!!loop->pred(text[count]) != loop->result)
            break;
    }

    return_error_if_fail(count >= loop->n, E_PARSER_FAIL, -1);

    if(parser_stack_push_match(stack, input, count))
        return error_pass(), -1;

    return count;
}

parser_ct parser_while_min(size_t n, ctype_pred_cb pred)
{
    parser_ctx_loop_st *loop;

    assert(pred);

    if(!n)
        return error_pass_ptr(parser_while(pred));

    if(!(loop = calloc(1, sizeof(parser_ctx_loop_st))))
        return error_wrap_last_errno(calloc), NULL;

    loop->n         = n;
    loop->pred      = pred;
    loop->result    = true;

    return error_pass_ptr(parser_new(parser_parse_loop, loop, free));
}

/// Parse callback for parser_while().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_while(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    ctype_pred_cb pred      = POINTER_TO_FUNC(ctx, ctype_pred_cb);
    parser_ctx_loop_st loop = { .n = 0, .pred = pred, .result = true };

    return error_pass_int(parser_parse_loop(input, size, &loop, stack, state));
}

parser_ct parser_while(ctype_pred_cb pred)
{
    assert(pred);

    return error_pass_ptr(parser_new(parser_parse_while, FUNC_TO_POINTER(pred), NULL));
}

parser_ct parser_until_min(size_t n, ctype_pred_cb pred)
{
    parser_ctx_loop_st *loop;

    assert(pred);

    if(!n)
        return error_pass_ptr(parser_until(pred));

    if(!(loop = calloc(1, sizeof(parser_ctx_loop_st))))
        return error_wrap_last_errno(calloc), NULL;

    loop->n         = n;
    loop->pred      = pred;
    loop->result    = false;

    return error_pass_ptr(parser_new(parser_parse_loop, loop, free));
}

/// Parse callback for parser_until().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_until(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    ctype_pred_cb pred      = POINTER_TO_FUNC(ctx, ctype_pred_cb);
    parser_ctx_loop_st loop = { .n = 0, .pred = pred, .result = false };

    return error_pass_int(parser_parse_loop(input, size, &loop, stack, state));
}

parser_ct parser_until(ctype_pred_cb pred)
{
    assert(pred);

    return error_pass_ptr(parser_new(parser_parse_until, FUNC_TO_POINTER(pred), NULL));
}

parser_ct parser_digits(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isdigit));
}

parser_ct parser_not_digits(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isdigit));
}

parser_ct parser_bdigits(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isbdigit));
}

parser_ct parser_not_bdigits(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isbdigit));
}

parser_ct parser_odigits(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isodigit));
}

parser_ct parser_not_odigits(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isodigit));
}

parser_ct parser_xdigits(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isxdigit));
}

parser_ct parser_not_xdigits(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isxdigit));
}

parser_ct parser_lxdigits(size_t n)
{
    return error_pass_ptr(parser_while_min(n, islxdigit));
}

parser_ct parser_not_lxdigits(size_t n)
{
    return error_pass_ptr(parser_until_min(n, islxdigit));
}

parser_ct parser_uxdigits(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isuxdigit));
}

parser_ct parser_not_uxdigits(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isuxdigit));
}

parser_ct parser_lowers(size_t n)
{
    return error_pass_ptr(parser_while_min(n, islower));
}

parser_ct parser_not_lowers(size_t n)
{
    return error_pass_ptr(parser_until_min(n, islower));
}

parser_ct parser_uppers(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isupper));
}

parser_ct parser_not_uppers(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isupper));
}

parser_ct parser_alnums(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isalnum));
}

parser_ct parser_not_alnums(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isalnum));
}

parser_ct parser_alphas(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isalpha));
}

parser_ct parser_not_alphas(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isalpha));
}

parser_ct parser_asciis(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isascii));
}

parser_ct parser_not_asciis(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isascii));
}

parser_ct parser_blanks(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isblank));
}

parser_ct parser_not_blanks(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isblank));
}

parser_ct parser_cntrls(size_t n)
{
    return error_pass_ptr(parser_while_min(n, iscntrl));
}

parser_ct parser_not_cntrls(size_t n)
{
    return error_pass_ptr(parser_until_min(n, iscntrl));
}

parser_ct parser_graphs(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isgraph));
}

parser_ct parser_not_graphs(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isgraph));
}

parser_ct parser_prints(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isprint));
}

parser_ct parser_not_prints(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isprint));
}

parser_ct parser_puncts(size_t n)
{
    return error_pass_ptr(parser_while_min(n, ispunct));
}

parser_ct parser_not_puncts(size_t n)
{
    return error_pass_ptr(parser_until_min(n, ispunct));
}

parser_ct parser_whitespaces(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isspace));
}

parser_ct parser_not_whitespaces(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isspace));
}

parser_ct parser_signs(size_t n)
{
    return error_pass_ptr(parser_while_min(n, issign));
}

parser_ct parser_not_signs(size_t n)
{
    return error_pass_ptr(parser_until_min(n, issign));
}

parser_ct parser_words(size_t n)
{
    return error_pass_ptr(parser_while_min(n, isword));
}

parser_ct parser_not_words(size_t n)
{
    return error_pass_ptr(parser_until_min(n, isword));
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
static ssize_t parser_parse_string_accept(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char **list = ctx;
    ssize_t rc;

    assert(*list);

    for(; *list; list++)
    {
        if((rc = parser_parse_string(input, size, (void *)*list, stack, state)) >= 0)
            return rc;

        if(!error_check(0, 1, E_PARSER_FAIL))
            return error_pass(), -1;
    }

    return error_pass(), -1;
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
