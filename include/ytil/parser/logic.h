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


/// New parser asserting success of a parser.
///
/// Execute sub parser.
/// Discard matched input.
/// Discard results.
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_assert(parser_ct p);

/// New parser asserting success of a parser and lifting a value.
///
/// Execute sub parser.
/// Discard results.
/// Discard matched input.
/// Push a value onto the stack.
///
/// Equivalent: parser_and(2, p, parser_lift(type, data, size, dtor))
///
/// \param p        parser
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_assert_lift(parser_ct p, const char *type, const void *data, size_t size, parser_dtor_cb dtor);

/// New parser asserting success of a parser and lifting a pointer value.
///
/// Execute sub parser.
/// Discard results.
/// Discard matched input.
/// Push a pointer value onto the stack.
///
/// Equivalent: parser_and(2, p, parser_lift_p(type, ptr, dtor))
///
/// \param p        parser
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_assert_lift_p(parser_ct p, const char *type, const void *ptr, parser_dtor_cb dtor);

/// New parser asserting success of a parser and lifting values.
///
/// Execute sub parser.
/// Discard results.
/// Disacrd matched input.
/// Push values onto the stack via \p lift callback.
///
/// Equivalent: parser_and(2, p, parser_lift_f(lift, ctx, dtor))
///
/// \param p        parser
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_assert_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor);

/// New parser executing a parser and dropping its results.
///
/// Execute sub parser.
/// Discard results.
/// Keep matched input.
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_drop(parser_ct p);

/// New parser executing a parser, dropping its results and lifting a value.
///
/// Execute sub parser.
/// Discard results.
/// Keep matched input.
/// Push a value onto the stack.
///
/// Equivalent: parser_seq(2, parser_drop(p), parser_lift(type, data, size, dtor))
///
/// \param p        parser
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_drop_lift(parser_ct p, const char *type, const void *data, size_t size, parser_dtor_cb dtor);

/// New parser executing a parser, dropping its results and lifting a pointer value.
///
/// Execute sub parser.
/// Discard results.
/// Keep matched input.
/// Push a pointer value onto the stack.
///
/// Equivalent: parser_seq(2, parser_drop(p), parser_lift_p(type, ptr, dtor))
///
/// \param p        parser
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_drop_lift_p(parser_ct p, const char *type, const void *ptr, parser_dtor_cb dtor);

/// New parser executing a parser, dropping its results and lifting values if it fails.
///
/// Execute sub parser.
/// Discard results.
/// Keep matched input.
/// Push values onto the stack via \p lift callback.
///
/// Equivalent: parser_seq(2, parser_drop(p), parser_lift_f(lift, ctx, dtor))
///
/// \param p        parser
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_drop_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor);

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

/// New parser inverting the status of a parser and lifting a value if it fails.
///
/// Execute sub parser.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// If sub parser fails, push a value onto the stack.
///
/// Equivalent: parser_and(2, parser_not(p), parser_lift(type, data, size, dtor))
///
/// \param p        parser
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lift(parser_ct p, const char *type, const void *data, size_t size, parser_dtor_cb dtor);

/// New parser inverting the status of a parser and lifting a pointer value if it fails.
///
/// Execute sub parser.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// If sub parser fails, push a pointer value onto the stack.
///
/// Equivalent: parser_and(2, parser_not(p), parser_lift_p(type, ptr, dtor))
///
/// \param p        parser
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lift_p(parser_ct p, const char *type, const void *ptr, parser_dtor_cb dtor);

/// New parser inverting the status of a parser and lifting values if it fails.
///
/// Execute sub parser.
/// Succeed if sub parser fails and fail if sub parser suceeds.
/// If sub parser fails, push values onto the stack via \p lift callback.
///
/// Equivalent: parser_and(2, parser_not(p), parser_lift_f(lift, ctx, dtor))
///
/// \param p        parser
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor);

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

/// New parser trying to execute a parser and lifting a value if it fails.
///
/// Execute sub parser once.
/// Sub parser may fail or succeed.
/// If sub parser fails, push a value onto the stack.
///
/// Equivalent: parser_or(2, p, parser_lift(type, data, size, dtor))
///
/// \param p        parser
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe_lift(parser_ct p, const char *type, const void *data, size_t size, parser_dtor_cb dtor);

/// New parser trying to execute a parser and lifting a pointer value if it fails.
///
/// Execute sub parser once.
/// Sub parser may fail or succeed.
/// If sub parser fails, push a pointer value onto the stack.
///
/// Equivalent: parser_or(2, p, parser_lift_p(type, ptr, dtor))
///
/// \param p        parser
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe_lift_p(parser_ct p, const char *type, const void *ptr, parser_dtor_cb dtor);

/// New parser trying to execute a parser and lifting values if it fails.
///
/// Execute sub parser once.
/// Sub parser may fail or succeed.
/// If sub parser fails, push values onto the stack via \p lift callback.
///
/// Equivalent: parser_or(2, p, parser_lift_f(lift, ctx, dtor))
///
/// \param p        parser
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_maybe_lift_f(parser_ct p, parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor);

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
