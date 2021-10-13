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

#include <ytil/parser/char.h>
#include <ytil/parser/null.h>
#include <ytil/def.h>
#include <string.h>

/// default error type for char parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


/// Parse callback for parser_any().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_any(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    return_error_if_fail(size, E_PARSER_FAIL, -1);

    if(parser_stack_push(stack, "char", input, sizeof(char), NULL))
        return error_pass(), -1;

    return 1;
}

parser_ct parser_any(void)
{
    return error_pass_ptr(parser_new(parser_parse_any, NULL, NULL));
}

/// Parse callback for parser_char().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_char(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char *text    = input;
    char c              = POINTER_TO_VALUE(ctx, char);

    return_error_if_fail(size && text[0] == c, E_PARSER_FAIL, -1);

    if(parser_stack_push(stack, "char", text, sizeof(char), NULL))
        return error_pass(), -1;

    return 1;
}

parser_ct parser_char(char c)
{
    return error_pass_ptr(parser_new(parser_parse_char, VALUE_TO_POINTER(c), NULL));
}

/// Parse callback for parser_not_char().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_not_char(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char *text    = input;
    char c              = POINTER_TO_VALUE(ctx, char);

    return_error_if_fail(size && text[0] != c, E_PARSER_FAIL, -1);

    if(parser_stack_push(stack, "char", text, sizeof(char), NULL))
        return error_pass(), -1;

    return 1;
}

parser_ct parser_not_char(char c)
{
    return error_pass_ptr(parser_new(parser_parse_not_char, VALUE_TO_POINTER(c), NULL));
}

/// Parse callback for parser_pred().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_pred(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char *text    = input;
    ctype_pred_cb pred  = POINTER_TO_FUNC(ctx, ctype_pred_cb);

    return_error_if_fail(size && pred(text[0]), E_PARSER_FAIL, -1);

    if(parser_stack_push(stack, "char", text, sizeof(char), NULL))
        return error_pass(), -1;

    return 1;
}

parser_ct parser_pred(ctype_pred_cb pred)
{
    assert(pred);

    return error_pass_ptr(parser_new(parser_parse_pred, FUNC_TO_POINTER(pred), NULL));
}

/// Parse callback for parser_not_pred().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_not_pred(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char *text    = input;
    ctype_pred_cb pred  = POINTER_TO_FUNC(ctx, ctype_pred_cb);

    return_error_if_fail(size && !pred(text[0]), E_PARSER_FAIL, -1);

    if(parser_stack_push(stack, "char", text, sizeof(char), NULL))
        return error_pass(), -1;

    return 1;
}

parser_ct parser_not_pred(ctype_pred_cb pred)
{
    assert(pred);

    return error_pass_ptr(parser_new(parser_parse_not_pred, FUNC_TO_POINTER(pred), NULL));
}

parser_ct parser_digit(void)
{
    return error_pass_ptr(parser_pred(isdigit));
}

parser_ct parser_not_digit(void)
{
    return error_pass_ptr(parser_not_pred(isdigit));
}

parser_ct parser_bdigit(void)
{
    return error_pass_ptr(parser_pred(isbdigit));
}

parser_ct parser_not_bdigit(void)
{
    return error_pass_ptr(parser_not_pred(isbdigit));
}

parser_ct parser_odigit(void)
{
    return error_pass_ptr(parser_pred(isodigit));
}

parser_ct parser_not_odigit(void)
{
    return error_pass_ptr(parser_not_pred(isodigit));
}

parser_ct parser_xdigit(void)
{
    return error_pass_ptr(parser_pred(isxdigit));
}

parser_ct parser_not_xdigit(void)
{
    return error_pass_ptr(parser_not_pred(isxdigit));
}

parser_ct parser_lxdigit(void)
{
    return error_pass_ptr(parser_pred(islxdigit));
}

parser_ct parser_not_lxdigit(void)
{
    return error_pass_ptr(parser_not_pred(islxdigit));
}

parser_ct parser_uxdigit(void)
{
    return error_pass_ptr(parser_pred(isuxdigit));
}

parser_ct parser_not_uxdigit(void)
{
    return error_pass_ptr(parser_not_pred(isuxdigit));
}

parser_ct parser_lower(void)
{
    return error_pass_ptr(parser_pred(islower));
}

parser_ct parser_not_lower(void)
{
    return error_pass_ptr(parser_not_pred(islower));
}

parser_ct parser_upper(void)
{
    return error_pass_ptr(parser_pred(isupper));
}

parser_ct parser_not_upper(void)
{
    return error_pass_ptr(parser_not_pred(isupper));
}

parser_ct parser_alnum(void)
{
    return error_pass_ptr(parser_pred(isalnum));
}

parser_ct parser_not_alnum(void)
{
    return error_pass_ptr(parser_not_pred(isalnum));
}

parser_ct parser_alpha(void)
{
    return error_pass_ptr(parser_pred(isalpha));
}

parser_ct parser_not_alpha(void)
{
    return error_pass_ptr(parser_not_pred(isalpha));
}

parser_ct parser_ascii(void)
{
    return error_pass_ptr(parser_pred(isascii));
}

parser_ct parser_not_ascii(void)
{
    return error_pass_ptr(parser_not_pred(isascii));
}

parser_ct parser_blank(void)
{
    return error_pass_ptr(parser_pred(isblank));
}

parser_ct parser_not_blank(void)
{
    return error_pass_ptr(parser_not_pred(isblank));
}

parser_ct parser_cntrl(void)
{
    return error_pass_ptr(parser_pred(iscntrl));
}

parser_ct parser_not_cntrl(void)
{
    return error_pass_ptr(parser_not_pred(iscntrl));
}

parser_ct parser_graph(void)
{
    return error_pass_ptr(parser_pred(isgraph));
}

parser_ct parser_not_graph(void)
{
    return error_pass_ptr(parser_not_pred(isgraph));
}

parser_ct parser_print(void)
{
    return error_pass_ptr(parser_pred(isprint));
}

parser_ct parser_not_print(void)
{
    return error_pass_ptr(parser_not_pred(isprint));
}

parser_ct parser_punct(void)
{
    return error_pass_ptr(parser_pred(ispunct));
}

parser_ct parser_not_punct(void)
{
    return error_pass_ptr(parser_not_pred(ispunct));
}

parser_ct parser_whitespace(void)
{
    return error_pass_ptr(parser_pred(isspace));
}

parser_ct parser_not_whitespace(void)
{
    return error_pass_ptr(parser_not_pred(isspace));
}

parser_ct parser_sign(void)
{
    return error_pass_ptr(parser_pred(issign));
}

parser_ct parser_not_sign(void)
{
    return error_pass_ptr(parser_not_pred(issign));
}

parser_ct parser_word(void)
{
    return error_pass_ptr(parser_pred(isword));
}

parser_ct parser_not_word(void)
{
    return error_pass_ptr(parser_not_pred(isword));
}

/// Parse callback for parser_accept().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_accept(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char *text = input, *accept = ctx;

    return_error_if_fail(size && strchr(accept, text[0]), E_PARSER_FAIL, -1);

    if(parser_stack_push(stack, "char", text, sizeof(char), NULL))
        return error_pass(), -1;

    return 1;
}

parser_ct parser_accept(const char *accept)
{
    assert(accept);

    if(!accept[0])
        return error_pass_ptr(parser_fail());

    if(!accept[1])
        return error_pass_ptr(parser_char(accept[0]));

    return error_pass_ptr(parser_new(parser_parse_accept, accept, NULL));
}

/// Parse callback for parser_reject().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_reject(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char *text = input, *reject = ctx;

    return_error_if_fail(size && !strchr(reject, text[0]), E_PARSER_FAIL, -1);

    if(parser_stack_push(stack, "char", text, sizeof(char), NULL))
        return error_pass(), -1;

    return 1;
}

parser_ct parser_reject(const char *reject)
{
    assert(reject);

    if(!reject[0])
        return error_pass_ptr(parser_any());

    if(!reject[1])
        return error_pass_ptr(parser_not_char(reject[0]));

    return error_pass_ptr(parser_new(parser_parse_reject, reject, NULL));
}

/// Parse callback for parser_range().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_range(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char *text    = input;
    int range           = POINTER_TO_VALUE(ctx, int);
    char start          = range & 0xff;
    char end            = (range >> 8) & 0xff;

    return_error_if_fail(size && RANGE(text[0], start, end), E_PARSER_FAIL, -1);

    if(parser_stack_push(stack, "char", text, sizeof(char), NULL))
        return error_pass(), -1;

    return 1;
}

parser_ct parser_range(char start, char end)
{
    int range = start | (end << 8);

    assert(start <= end);

    if(start == end)
        return error_pass_ptr(parser_char(start));

    return error_pass_ptr(parser_new(parser_parse_range, VALUE_TO_POINTER(range), NULL));
}

/// Parse callback for parser_not_range().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_not_range(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char *text    = input;
    int range           = POINTER_TO_VALUE(ctx, int);
    char start          = range & 0xff;
    char end            = (range >> 8) & 0xff;

    return_error_if_fail(size && !RANGE(text[0], start, end), E_PARSER_FAIL, -1);

    if(parser_stack_push(stack, "char", text, sizeof(char), NULL))
        return error_pass(), -1;

    return 1;
}

parser_ct parser_not_range(char start, char end)
{
    int range = start | (end << 8);

    assert(start <= end);

    if(start == end)
        return error_pass_ptr(parser_not_char(start));

    return error_pass_ptr(
        parser_new(parser_parse_not_range, VALUE_TO_POINTER(range), NULL));
}

/// Parse callback for parser_escape().
///
/// \implements parser_parse_cb
static ssize_t parser_parse_escape(const void *input, size_t size, void *ctx, parser_stack_ct stack, void *state)
{
    const char *text    = input;
    char esc            = POINTER_TO_VALUE(ctx, char);

    return_error_if_fail(size >= 2 && text[0] == esc, E_PARSER_FAIL, -1);

    if(parser_stack_push(stack, "char", &text[1], sizeof(char), NULL))
        return error_pass(), -1;

    return 2;
}

parser_ct parser_escape(char esc)
{
    return error_pass_ptr(parser_new(parser_parse_escape, VALUE_TO_POINTER(esc), NULL));
}
