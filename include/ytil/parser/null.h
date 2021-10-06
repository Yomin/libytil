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

/// New parser matching end of input.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_end(void);

/// New parser which pushes a value onto the stack and succeeds.
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
/// \param type     value type
/// \param ptr      pointer value, must not be NULL
/// \param dtor     value destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_lift_p(const char *type, const void *ptr, parser_dtor_cb dtor);

/// New parser which pushes values onto the stack and succeeds.
///
/// \param lift     callback to push values onto the stack
/// \param ctx      \p lift context
/// \param dtor     \p ctx destructor
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_lift_f(parser_lift_cb lift, const void *ctx, parser_dtor_cb dtor);


#endif
