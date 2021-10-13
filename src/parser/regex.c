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

#include <ytil/parser/regex.h>
#include <ytil/parser/sub.h>
#include <ytil/parser/repeat.h>
#include <ytil/parser/char.h>
#include <ytil/parser/string.h>
#include <ytil/parser/logic.h>
#include <ytil/parser/num.h>


/// default error type for regex parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_REGEX


static parser_ct parser_regex_dot(void)
{
    return error_pass_ptr(parser_char('.'));
}

static parser_ct parser_regex_class(void)
{
    return error_pass_ptr(parser_seq(2,
        parser_drop(parser_string("[:")),
        parser_assert(parser_seq(2,
            parser_string_accept(14,
                "alnum", "alpha", "ascii", "blank", "cntrl", "digit", "graph",
                "lower", "print", "punct", "space", "upper", "word", "xdigit"),
            parser_drop(parser_string(":]"))))));
}

static parser_ct parser_regex_range(void)
{
    return error_pass_ptr(parser_seq(3,
        parser_reject("-]"),
        parser_char('-'),
        parser_reject("-]")));
}

static parser_ct parser_regex_bracket(void)
{
    parser_ct accept, bracket;

    accept = parser_ref(parser_or(3,
        parser_regex_class(),
        parser_regex_range(),
        parser_not_char(']')));

    if(!accept)
        return error_pass(), NULL;

    bracket = parser_seq(2,
        parser_drop(parser_char('[')),
        parser_assert_e(parser_seq(3,
            parser_maybe(parser_char('^')),
            parser_or(2,
                parser_seq(2,
                    parser_char(']'),
                    parser_many(accept)),
                parser_min1(accept)),
            parser_drop(parser_char(']'))),
            "foo", E_REGEX_INVALID_BRACKET));

    parser_unref(accept);

    return error_pass_ptr(bracket);
}

static parser_ct parser_regex_group_no_capture(parser_ct expr)
{
    return error_pass_ptr(parser_seq(2,
        parser_drop(parser_char(':')),
        expr));
}

static parser_ct parser_regex_group_named_capture(parser_ct expr)
{
    return error_pass_ptr(parser_seq(3,
        parser_maybe_drop(parser_char('P')),
        parser_seq(3,
            parser_drop(parser_char('<')),
            parser_min1(parser_not_char('>')),
            parser_drop(parser_char('>'))),
        expr));
}

static parser_ct parser_regex_options(void)
{
    parser_ct option, options;

    if(!(option = parser_ref(parser_min1(parser_accept("icsmxtn")))))
        return error_pass(), NULL;

    options = parser_seq(2,
        option,
        parser_maybe(parser_seq(2,
            parser_char('-'),
            option)));

    parser_unref(option);

    return error_pass_ptr(options);
}

static parser_ct parser_regex_group(parser_ct expr)
{
    return error_pass_ptr(parser_seq(2,
        parser_drop(parser_char('(')),
        parser_assert(parser_seq(2,
            parser_or(2,
                parser_seq(2,
                    parser_drop(parser_char('?')),
                    parser_assert(parser_or(3,
                        parser_regex_group_no_capture(expr),
                        parser_regex_group_named_capture(expr),
                        parser_regex_options()))),
                expr),
            parser_drop(parser_char(')'))))));
}

static parser_ct parser_regex_token(parser_ct expr)
{
    return error_pass_ptr(parser_or(5,
        parser_min1(parser_reject("|.+*?[](){}^$")),
        parser_escape('\\'),
        parser_regex_dot(),
        parser_regex_bracket(),
        parser_regex_group(expr)));
}

static parser_ct parser_regex_minmax(void)
{
    return error_pass_ptr(parser_seq(2,
        parser_drop(parser_char('{')),
        parser_assert(parser_seq(3,
            parser_uint(),
            parser_maybe(parser_seq(2,
                parser_char(','),
                parser_maybe(parser_uint()))),
            parser_drop(parser_char('}'))))));
}

static parser_ct parser_regex_quantifier(void)
{
    return error_pass_ptr(parser_or(2,
        parser_accept("+*?"),
        parser_regex_minmax()));
}

static parser_ct parser_regex_term(parser_ct expr)
{
    return error_pass_ptr(parser_min1(parser_seq(2,
        parser_regex_token(expr),
        parser_maybe(parser_regex_quantifier()))));
}

parser_ct parser_regex(void)
{
    parser_ct term, regex;

    if(!(regex = parser_new_link()))
        return error_pass(), NULL;

    if(!(term = parser_ref(parser_regex_term(regex))))
        return error_pass(), parser_unref(regex), NULL;

    regex = parser_link(regex, parser_seq(2,
        term,
        parser_many(parser_seq(2,
            parser_drop(parser_char('|')),
            parser_assert(term)))));

    parser_unref(term);

    return error_pass_ptr(regex);
}
