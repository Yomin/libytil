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

#ifndef YTIL_PARSER_NUM_H_INCLUDED
#define YTIL_PARSER_NUM_H_INCLUDED

#include <ytil/parser/parser.h>


/// New parser which matches a signed integer.
///
/// \par Equivalent
///     parser_seq(2, parser_maybe(parser_sign()), parser_digits())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_num(void);

/// New parser which parses a signed integer.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_int(void);

/// New parser which parses an unsigned signed integer.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_uint(void);

/// New parser which matches a floating point value.
///
/// \par Equivalent
///     parser_seq(3,
///         parser_num(),
///         parser_maybe(parser_seq(2, parser_char('.'), parser_digits())),
///         parser_maybe(parser_seq(2, parser_accept("eE"), parser_num())))
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_real(void);

/// New parser which parses a floating point value.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_float(void);


#endif
