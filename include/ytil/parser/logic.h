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

#ifndef YTIL_PARSER_LOGIC_H_INCLUDED
#define YTIL_PARSER_LOGIC_H_INCLUDED

#include <ytil/parser/parser.h>
#include <ytil/parser/null.h>
#include <stdarg.h>


/// New parser which checks for success of a parser.
///
/// Positive lookahead.
/// Discard matched input.
/// Discard results.
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_check(parser_ct p);

/// New parser which checks for success of a parser and lifts a value.
///
/// Positive lookahead.
/// Discard results.
/// Discard matched input.
/// Push a value onto the stack.
///
/// \par Equivalent
///     parser_and(2, p, parser_lift(type, data, size))
///
/// \param p        parser
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_check_lift(parser_ct p, const char *type, const void *data, size_t size);

/// New parser which checks for success of a parser and lifts a pointer value.
///
/// Positive lookahead.
/// Discard results.
/// Discard matched input.
/// Push a pointer value onto the stack.
///
/// \par Equivalent
///     parser_and(2, p, parser_lift_p(type, ptr))
///
/// \param p        parser
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_check_lift_p(parser_ct p, const char *type, const void *ptr);

/// New parser which checks for success of a parser and lifts values.
///
/// Positive lookahead.
/// Discard results.
/// Discard matched input.
/// Push values onto the stack via \p lift callback.
///
/// \par Equivalent
///     parser_and(2, p, parser_lift_f(lift, ctx, dtor))
///
/// \param p        parser
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_check_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor);

/// New parser which executs a parser and drops its results.
///
/// Discard results.
/// Keep matched input.
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_drop(parser_ct p);

/// New parser which executs a parser, drops its results and lifts a value.
///
/// Discard results.
/// Keep matched input.
/// Push a value onto the stack.
///
/// \par Equivalent
///     parser_seq(2, parser_drop(p), parser_lift(type, data, size))
///
/// \param p        parser
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_drop_lift(parser_ct p, const char *type, const void *data, size_t size);

/// New parser which executs a parser, drops its results and lifts a pointer value.
///
/// Discard results.
/// Keep matched input.
/// Push a pointer value onto the stack.
///
/// \par Equivalent
///     parser_seq(2, parser_drop(p), parser_lift_p(type, ptr))
///
/// \param p        parser
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_drop_lift_p(parser_ct p, const char *type, const void *ptr);

/// New parser which executs a parser, drops its results and lifts values if it fails.
///
/// Discard results.
/// Keep matched input.
/// Push values onto the stack via \p lift callback.
///
/// \par Equivalent
///     parser_seq(2, parser_drop(p), parser_lift_f(lift, ctx, dtor))
///
/// \param p        parser
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_drop_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor);

/// New parser which inverts the status of a parser.
///
/// Negative lookahead.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// No results produced.
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not(parser_ct p);

/// New parser which inverts the status of a parser and lifts a value if it fails.
///
/// Negative lookahead.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// If sub parser fails, push a value onto the stack.
///
/// \par Equivalent
///     parser_and(2, parser_not(p), parser_lift(type, data, size))
///
/// \param p        parser
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lift(parser_ct p, const char *type, const void *data, size_t size);

/// New parser which inverts the status of a parser and lifts a pointer value if it fails.
///
/// Negative lookahead.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// If sub parser fails, push a pointer value onto the stack.
///
/// \par Equivalent
///     parser_and(2, parser_not(p), parser_lift_p(type, ptr))
///
/// \param p        parser
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lift_p(parser_ct p, const char *type, const void *ptr);

/// New parser which inverts the status of a parser and lifts values if it fails.
///
/// Negative lookahead.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// If sub parser fails, push values onto the stack via \p lift callback.
///
/// \par Equivalent
///     parser_and(2, parser_not(p), parser_lift_f(lift, ctx, dtor))
///
/// \param p        parser
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor);

/// New parser which tries to execute a parser.
///
/// Sub parser may fail or succeed.
///
/// \par Equivalent
///     parser_or(2, p, parser_success())
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe(parser_ct p);

/// New parser which tries to execute a parser and drops its results.
///
/// Sub parser may fail or succeed.
/// Discard results.
/// Keep matched input.
///
/// \par Equivalent
///     parser_or(2, parser_drop(p), parser_success())
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe_drop(parser_ct p);

/// New parser which tries to execute a parser and lifts a value if it fails.
///
/// Sub parser may fail or succeed.
/// If sub parser fails, push a value onto the stack.
///
/// \par Equivalent
///     parser_or(2, p, parser_lift(type, data, size))
///
/// \param p        parser
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe_lift(parser_ct p, const char *type, const void *data, size_t size);

/// New parser which tries to execute a parser and lifts a pointer value if it fails.
///
/// Sub parser may fail or succeed.
/// If sub parser fails, push a pointer value onto the stack.
///
/// \par Equivalent
///     parser_or(2, p, parser_lift_p(type, ptr))
///
/// \param p        parser
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe_lift_p(parser_ct p, const char *type, const void *ptr);

/// New parser which tries to execute a parser and lifts values if it fails.
///
/// Sub parser may fail or succeed.
/// If sub parser fails, push values onto the stack via \p lift callback.
///
/// \par Equivalent
///     parser_or(2, p, parser_lift_f(lift, ctx, dtor))
///
/// \param p        parser
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor);

/// New parser which combines a number of parsers with logical AND.
///
/// Execute parsers on same input.
/// All parsers need to succeed.
/// Only the last parser may produce results.
///
/// \par Equivalent
///     parser_seq(n, parser_check(p1), parser_check(p2), ..., pn)
///
/// \param n    number of parsers
/// \param ...  variadic list of parser_ct
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_and(size_t n, ...);

/// New parser which combines a number of parsers with logical AND.
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

/// New parser which combines a number of parsers with logical OR.
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

/// New parser which combines a number of parsers with logical OR.
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
