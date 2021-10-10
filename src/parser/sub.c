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

#include <ytil/parser/sub.h>
#include <ytil/parser/null.h>
#include <ytil/def.h>
#include <stdlib.h>

/// default error type for sub parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


/// Free parser context with sub parser as context.
///
/// \implements parser_dtor_cb
static void parser_dtor_parser(void *ctx)
{
    parser_unref(ctx);
}

parser_ct parser_new_parser(parser_parse_cb parse, parser_ct p)
{
    if(!p)
        return error_pass(), NULL;

    return error_pass_ptr(parser_new(parse, parser_ref_sink(p), parser_dtor_parser));
}

parser_ct *parser_list_new_v(size_t n, va_list parsers)
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
            goto sink;
        }
    }

    va_end(ap);

    if(!(list = calloc(n + 1, sizeof(parser_ct))))
    {
        error_wrap_last_errno(calloc);
        goto sink;
    }

    for(i = 0; i < n; i++)
        list[i] = parser_ref_sink(va_arg(parsers, parser_ct));

    return list;

sink:
    for(i = 0; i < n; i++)
    {
        if((p = va_arg(parsers, parser_ct)))
            parser_sink(p);
    }

    return NULL;
}

void parser_list_free(parser_ct *list)
{
    parser_ct *p;

    for(p = list; *p; p++)
        parser_unref(*p);

    free(list);
}

/// Free parser context of parser_new_parser_list_v().
///
/// \implements parser_dtor_cb
static void parser_dtor_new_parser_list_v(void *ctx)
{
    parser_list_free(ctx);
}

parser_ct parser_new_parser_list_v(parser_parse_cb parse, size_t n, va_list parsers)
{
    parser_ct *list;

    if(!(list = parser_list_new_v(n, parsers)))
        return error_pass(), NULL;

    return error_pass_ptr(parser_new(parse, list, parser_dtor_new_parser_list_v));
}

/// Parse callback for parser_seq().
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
    if(!n)
        return error_pass_ptr(parser_success());

    if(n == 1)
        return error_pass_ptr(va_arg(parsers, parser_ct));

    return error_pass_ptr(parser_new_parser_list_v(parser_parse_seq, n, parsers));
}
