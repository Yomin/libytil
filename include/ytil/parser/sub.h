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

#ifndef YTIL_PARSER_SUB_H_INCLUDED
#define YTIL_PARSER_SUB_H_INCLUDED

#include <ytil/parser/parser.h>
#include <stdarg.h>


/// Create new parser with sub parser as context.
///
/// The parser takes ownership of the sub parser.
///
/// parser context type: parser_ct
///
/// \param parse    parse callback
/// \param p        sub parser
///
/// \returns                    new parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_new_parser(parser_parse_cb parse, parser_ct p);

/// Create parser list from variadic argument list.
///
/// Takes ownership of all parsers.
///
/// \param n        number of sub parsers
/// \param parsers  variadic list of parser_ct
///
/// \returns                    NULL terminated list of parser_ct
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct *parser_list_new_v(size_t n, va_list parsers);

/// Free parser list.
///
/// \param list     NUll terminated list of parser_ct
void parser_list_free(parser_ct *list);

/// Create new parser with sub parser list as context.
///
/// The parser takes ownership of all sub parsers.
///
/// parser context type: NULL terminated list of parser_ct
///
/// \param parse    parse callback
/// \param n        number of sub parsers
/// \param parsers  variadic list of parser_ct
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_new_parser_list_v(parser_parse_cb parse, size_t n, va_list parsers);


/// New parser executing a number of parsers in sequence.
///
/// Execute parsers in sequence.
/// All parsers need to succeed.
/// All parsers may produce results.
///
/// \param n    number of parsers
/// \param ...  variadic list of parser_ct
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_seq(size_t n, ...);

/// New parser executing a number of parsers in sequence.
///
/// Execute parsers in sequence.
/// All parsers need to succeed.
///
/// \param n        number of parsers
/// \param parsers  variadic list of parser_ct
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_seq_v(size_t n, va_list parsers);


#endif
