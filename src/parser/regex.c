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
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


static parser_ct parser_regex_term(void);

static parser_ct parser_regex_group_no_capture(parser_ct term)
{
    return error_pass_ptr(parser_seq(2,
        parser_drop(parser_char(':')),
        term));
}

static parser_ct parser_regex_group_named_capture(parser_ct term)
{
    return error_pass_ptr(parser_seq(3,
        parser_maybe_drop(parser_char('P')),
        parser_seq(3,
            parser_drop(parser_char('<')),
            parser_min1(parser_not_char('>')),
            parser_drop(parser_char('>'))),
        term));
}

static parser_ct parser_regex_option(void)
{
    return error_pass_ptr(parser_seq(2,
        parser_min1(parser_accept("icsmxtn")),
        parser_maybe(parser_seq(2,
            parser_char('-'),
            parser_min1(parser_accept("icsmxtn"))))));
}

static parser_ct parser_regex_group(parser_ct term)
{
    return error_pass_ptr(parser_seq(2,
        parser_drop(parser_char('(')),
        parser_assert(parser_seq(2,
            parser_or(2,
                parser_seq(2,
                    parser_drop(parser_char('?')),
                    parser_assert(parser_or(3,
                        parser_regex_group_no_capture(term),
                        parser_regex_group_named_capture(term),
                        parser_regex_option())),
                term)),
            parser_drop(parser_char(')'))))));
}

static parser_ct parser_regex_class(void)
{
    return error_pass_ptr(parser_seq(3,
        parser_drop(parser_string("[:")),
        parser_string_accept(14,
            "alnum", "alpha", "ascii", "blank", "cntrl", "digit", "graph",
            "lower", "print", "punct", "space", "upper", "word", "xdigit"),
        parser_drop(parser_string(":]"))));
}

static parser_ct parser_regex_range(void)
{
    return error_pass_ptr(parser_seq(3,
        parser_word(),
        parser_char('-'),
        parser_word()));
}

static parser_ct parser_regex_bracket(void)
{
    return error_pass_ptr(parser_seq(3,
        parser_drop(parser_char('[')),
        parser_or(3,
            parser_regex_class(),
            parser_regex_range(),
            parser_min1(parser_not_char(']'))),
        parser_drop(parser_char(']'))));
}

static parser_ct parser_regex_token(parser_ct term)
{
    return error_pass_ptr(parser_or(3,
        parser_regex_group(term),
        parser_regex_bracket(),
        parser_min1(parser_any())));
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

static parser_ct parser_regex_term(void)
{
    parser_ct term, link;

    link    = parser_ref(parser_link());
    term    = parser_min1(parser_seq(2,
        parser_regex_token(link),
        parser_maybe(parser_regex_quantifier())));

    parser_link_set(link, term);
    parser_unref(link);

    return error_pass_ptr(term);
}

parser_ct parser_regex(void)
{
    parser_ct term, regex;

    term    = parser_ref(parser_regex_term());
    regex   = parser_seq(2,
        term,
        parser_many(parser_seq(2,
            parser_drop(parser_char('|')),
            parser_assert(term))));

    parser_unref(term);

    return error_pass_ptr(regex);
}
