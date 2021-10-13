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

#ifndef YTIL_PARSER_NULL_H_INCLUDED
#define YTIL_PARSER_NULL_H_INCLUDED

#include <ytil/parser/parser.h>


/// parser lift callback
///
/// \param stack    parse stack
/// \param ctx      callback context
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
typedef int (*parser_lift_cb)(parser_stack_ct stack, void *ctx);


/// Create new parser which lifts a value on success.
///
/// \note If this function fails, the callback context and the lift value
///       are immediately destroyed.
///
/// \par Equivalent
///     parser_seq(2,
///         parser_new(parse, parse_ctx, parse_dtor),
///         parser_lift(type, data, size, data_dtor))
///
/// \param parse        parse callback
/// \param parse_ctx    \p parse context
/// \param parse_dtor   \p parse_ctx destructor
/// \param type         lift value type
/// \param data         pointer to lift value data
/// \param size         lift value size
/// \param data_dtor    lift value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_new_lift_success(parser_parse_cb parse, const void *parse_ctx, parser_dtor_cb parse_dtor, const char *type, const void *data, size_t size, parser_dtor_cb data_dtor);

/// Create new parser which lifts a value on fail.
///
/// \note If this function fails, the callback context and the lift value
///       are immediately destroyed.
///
/// \par Equivalent
///     parser_or(2,
///         parser_new(parse, parse_ctx, parse_dtor),
///         parser_lift(type, data, size, data_dtor))
///
/// \param parse        parse callback
/// \param parse_ctx    \p parse context
/// \param parse_dtor   \p parse_ctx destructor
/// \param type         lift value type
/// \param data         pointer to lift value data
/// \param size         lift value size
/// \param data_dtor    lift value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_new_lift_fail(parser_parse_cb parse, const void *parse_ctx, parser_dtor_cb parse_dtor, const char *type, const void *data, size_t size, parser_dtor_cb data_dtor);

/// Create new parser which executes a lift CTOR on success.
///
/// \note If this function fails, the callback contexts are immediately destroyed.
///
/// \par Equivalent
///     parser_seq(2,
///         parser_new(parse, parse_ctx, parse_dtor),
///         parser_lift_f(lift, lift_ctx, lift_dtor))
///
/// \param parse        parse callback
/// \param parse_ctx    \p parse context
/// \param parse_dtor   \p parse_ctx destructor
/// \param lift         callback to push values onto the stack
/// \param lift_ctx     \p lift context
/// \param lift_dtor    \p lift_ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_new_lift_success_f(parser_parse_cb parse, const void *parse_ctx, parser_dtor_cb parse_dtor, parser_lift_cb lift, const void *lift_ctx, parser_dtor_cb lift_dtor);

/// Create new parser which executes a lift CTOR on fail.
///
/// \note If this function fails, the callback contexts are immediately destroyed.
///
/// \par Equivalent
///     parser_or(2,
///         parser_new(parse, parse_ctx, parse_dtor),
///         parser_lift_f(lift, lift_ctx, lift_dtor))
///
/// \param parse        parse callback
/// \param parse_ctx    \p parse context
/// \param parse_dtor   \p parse_ctx destructor
/// \param lift         callback to push values onto the stack
/// \param lift_ctx     \p lift context
/// \param lift_dtor    \p lift_ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_new_lift_fail_f(parser_parse_cb parse, const void *parse_ctx, parser_dtor_cb parse_dtor, parser_lift_cb lift, const void *lift_ctx, parser_dtor_cb lift_dtor);


/// New parser which always succeeds.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_success(void);

/// New parser which always fails.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_fail(void);

/// New parser which always aborts.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_abort(void);

/// New parser which always aborts and pushes an error of default type.
///
/// \param name     function name
/// \param code     error code
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
#define parser_abort_e(name, code) \
    parser_abort_es((name), ERROR_TYPE(DEFAULT), (code))

/// New parser which always aborts and pushes an error of specific type.
///
/// \param name     function name
/// \param type     error type
/// \param code     error code
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_abort_es(const char *name, const error_type_st *type, int code);

/// New parser which asserts success of a parser or aborts.
///
/// Abort if sub parser fails.
///
/// \par Equivalent
///     parser_or(p, parser_abort())
///
/// \param p    parser
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_assert(parser_ct p);

/// New parser which asserts success of a parser or aborts with an error of default type.
///
/// Abort with error if sub parser fails.
///
/// \par Equivalent
///     parser_or(p, parser_abort_e(name, code))
///
/// \param p        parser
/// \param name     function name
/// \param code     error code
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
#define parser_assert_e(p, name, code) \
    parser_assert_es((p), (name), ERROR_TYPE(DEFAULT), (code))

/// New parser which asserts success of a parser or aborts with an error of specific type.
///
/// Abort with error if sub parser fails.
///
/// \par Equivalent
///     parser_or(p, parser_abort_es(name, type, code))
///
/// \param p        parser
/// \param name     function name
/// \param type     error type
/// \param code     error code
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_assert_es(parser_ct p, const char *name, const error_type_st *type, int code);

/// New parser matching end of input.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_end(void);

/// New parser which pushes a value onto the stack and succeeds.
///
/// \note If this function fails, the lift value is immediately destroyed.
///
/// \param type     value type
/// \param data     pointer to value data
/// \param size     value size
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_lift(const char *type, const void *data, size_t size, parser_dtor_cb dtor);

/// New parser which pushes a pointer value onto the stack and succeeds.
///
/// \note If this function fails, the pointer lift value is immediately destroyed.
///
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_lift_p(const char *type, const void *ptr, parser_dtor_cb dtor);

/// New parser which pushes values onto the stack and succeeds.
///
/// \note If this function fails, the callback context is immediately destroyed.
///
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_lift_f(parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor);

/// New phony parser which executes a parser which it has no reference for.
///
/// Used for building self-reference parsers.
///
/// \note The parser is created with a real reference
///       which is expected to be consumed by parser_link().
///
/// \code
/// parser_ct parser_foo(void)
/// {
///     parser_ct foo;
///
///     if(!(foo = parser_new_link()))
///         return error_pass(), NULL;
///
///     return error_pass_ptr(parser_link(foo, parser_or(2, parser_bar(foo), parser_baz(foo))));
/// }
/// \endcode
///
/// \returns                    parser with real reference
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_new_link(void);

/// Link the phony parser to the actual parser.
///
/// \note This function consumes the reference introduced by parser_new_link().
///       If the link was not used or \p has failed and thus put no reference on \p link,
///       it is freed here.
///
/// \param link     link parser, must be valid
/// \param p        actual parser, may have failed
///
/// \returns p
parser_ct parser_link(parser_ct link, parser_const_ct p);


#endif
