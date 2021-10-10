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

#ifndef YTIL_PARSER_REPEAT_H_INCLUDED
#define YTIL_PARSER_REPEAT_H_INCLUDED

#include <ytil/parser/parser.h>


/// New parser executing a parser an exact number of times.
///
/// Execute sub parser n times.
/// Sub parser must succeed every time.
///
/// \param n    number of executions
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_repeat(size_t n, parser_ct p);

/// New parser executing a parser an exact number of times.
///
/// Execute sub parser n times.
/// Sub parser must succeed every time.
///
/// \param p    parser
///
/// \tparam n   [size_t] number of executions
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_repeat_x(parser_ct p);

/// New parser executing a parser at least a number of times.
///
/// Execute sub parser as many times as possible.
/// Sub parser must succeed at least the first n times.
///
/// \param n    minimum number of executions
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_min(size_t n, parser_ct p);

/// New parser executing a parser at least once.
///
/// Execute sub parser as many times as possible.
/// Sub parser must succeed at least once.
///
/// \par Equivalent
///     parser_min(1, p)
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_min1(parser_ct p);

/// New parser executing a parser as many times as possible.
///
/// Execute sub parser as many times as possible.
/// Sub parser may fail or succeed.
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_many(parser_ct p);

/// New parser executing a parser at least a minimum number of times
/// but at most a maximum number of times.
///
/// Execute sub parser up to \p max times.
/// Sub parser must succeed at least the first \p min times.
///
/// \param min  minimum number of executions
/// \param max  maximum number of executions
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_minmax(size_t min, size_t max, parser_ct p);

/// New parser executing a parser at most a number of times.
///
/// Execute sub parser up to n times.
/// Sub parser may fail or succeed.
///
/// \param n    maximum number of executions
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_max(size_t n, parser_ct p);


#endif
