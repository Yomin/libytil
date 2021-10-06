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

#ifndef YTIL_PARSER_COMBINATOR_H_INCLUDED
#define YTIL_PARSER_COMBINATOR_H_INCLUDED

#include <ytil/parser/parser.h>
#include <ytil/parser/null.h>
#include <stdarg.h>


/// New parser asserting success of a parser.
///
/// Execute sub parser.
/// Discard results if any produced.
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_assert(parser_ct p);

/// New parser inverting the status of a parser.
///
/// Execute sub parser.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// No results produced.
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not(parser_ct p);

/// New parser inverting the status of a parser
/// and pushing a value onto the stack if it fails.
///
/// Execute sub parser.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// If sub parser fails, push a value onto the stack.
///
/// Equivalent: parser_and(2, parser_not(p), parser_lift(type, data, size, dtor))
///
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
/// \param dtor     value destructor
/// \param p        parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lift(const char *type, const void *data, size_t size, parser_dtor_cb dtor, parser_ct p);

/// New parser inverting the status of a parser
/// and pushing a pointer value onto the stack if it fails.
///
/// Execute sub parser.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// If sub parser fails, push a pointer value onto the stack.
///
/// Equivalent: parser_and(2, parser_not(p), parser_lift_p(type, ptr, dtor))
///
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
/// \param dtor     value destructor
/// \param p        parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lift_p(const char *type, const void *ptr, parser_dtor_cb dtor, parser_ct p);

/// New parser inverting the status of a parser
/// and pushing values onto the stack if it fails.
///
/// Execute sub parser.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// If sub parser fails, push values onto the stack via \p lift callback.
///
/// Equivalent: parser_and(2, parser_not(p), parser_lift_f(lift, ctx, dtor))
///
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
/// \param p        parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lift_f(parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor, parser_ct p);

/// New parser trying to execute a parser.
///
/// Execute sub parser once.
/// Sub parser may fail or succeed.
///
/// Equivalent: parser_or(2, p, parser_success())
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe(parser_ct p);

/// New parser trying to execute a parser
/// and pushing a value onto the stack if it fails.
///
/// Execute sub parser once.
/// Sub parser may fail or succeed.
/// If sub parser fails, push a value onto the stack.
///
/// Equivalent: parser_or(2, p, parser_lift(type, data, size, dtor))
///
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
/// \param dtor     value destructor
/// \param p        parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe_lift(const char *type, const void *data, size_t size, parser_dtor_cb dtor, parser_ct p);

/// New parser trying to execute a parser
/// and pushing a pointer value onto the stack if it fails.
///
/// Execute sub parser once.
/// Sub parser may fail or succeed.
/// If sub parser fails, push a pointer value onto the stack.
///
/// Equivalent: parser_or(2, p, parser_lift_p(type, ptr, dtor))
///
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
/// \param dtor     value destructor
/// \param p        parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe_lift_p(const char *type, const void *ptr, parser_dtor_cb dtor, parser_ct p);

/// New parser trying to execute a parser
/// and pushing values onto the stack if it fails.
///
/// Execute sub parser once.
/// Sub parser may fail or succeed.
/// If sub parser fails, push values onto the stack via \p lift callback.
///
/// Equivalent: parser_or(2, p, parser_lift_f(lift, ctx, dtor))
///
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
/// \param p        parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe_lift_f(parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor, parser_ct p);

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

/// New parser executing a parser an exact number of times.
///
/// Execute sub parser n times.
/// Sub parser must succeed every time.
///
/// Equivalent: parser_minmax(n, n, p)
///
/// \param n    number of executions
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_repeat(size_t n, parser_ct p);

/// New parser executing a parser at least once.
///
/// Execute sub parser as many times as possible.
/// Sub parser must succeed at least once.
///
/// Equivalent: parser_min(1, p)
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_min1(parser_ct p);

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

/// New parser executing a parser at most a number of times.
///
/// Execute sub parser up to n times.
/// Sub parser may fail or succeed.
///
/// Equivalent: parser_minmax(0, n, p)
///
/// \param n    maximum number of executions
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_max(size_t n, parser_ct p);

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

/// New parser combining a number of parsers with logical AND.
///
/// Execute parsers on same input.
/// All parsers need to succeed.
/// Only the last parser may produce results.
///
/// Equivalent: parser_seq(n, parser_assert(p1), parser_assert(p2), ..., pn)
///
/// \param n    number of parsers
/// \param ...  variadic list of parser_ct
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_and(size_t n, ...);

/// New parser combining a number of parsers with logical AND.
///
/// Execute parsers on same input.
/// All parsers need to succeed.
/// Only the last parser may produce results.
///
/// \param n        number of parsers
/// \param parsers  variadic list of parser_ct
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_and_v(size_t n, va_list parsers);

/// New parser combining a number of parsers with logical OR.
///
/// Execute parsers on same input.
/// At least one parser needs to succeed.
/// The first parser to succeed may produce results.
///
/// \param n    number of parsers
/// \param ...  variadic list of parser_ct
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_or(size_t n, ...);

/// New parser combining a number of parsers with logical OR.
///
/// Execute parsers on same input.
/// At least one parser needs to succeed.
/// The first parser to succeed may produce results.
///
/// \param n        number of parsers
/// \param parsers  variadic list of parser_ct
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_or_v(size_t n, va_list parsers);


#endif
