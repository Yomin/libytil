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

#ifndef YTIL_GEN_REGEX_H_INCLUDED
#define YTIL_GEN_REGEX_H_INCLUDED

#include <ytil/gen/error.h>


/// regex error
typedef enum regex_error
{
    E_REGEX_INVALID_BRACKET,
} regex_error_id;

/// regex error type declaration
ERROR_DECLARE(REGEX);


struct regex_compiler;
typedef       struct regex_compiler *regex_compiler_ct;         ///< regex compiler type
typedef const struct regex_compiler *regex_compiler_const_ct;   ///< const regex compiler type

struct regex;
typedef       struct regex *regex_ct;       ///< regex type
typedef const struct regex *regex_const_ct; ///< const regex type


/// Create a new regular expression compiler.
///
/// \returns                    regex compiler
/// \retval NULL/E_GENERIC_OOM  out of memory
regex_compiler_ct regex_compiler_new(void);

/// Free regular expression compiler.
///
/// \param c    regex compiler
void regex_compiler_free(regex_compiler_ct c);

/// Compile a regular expression.
///
/// \param c        regex compiler
/// \param text     regular expression
/// \param len      \p text length
///
/// \returns                    regex
/// \retval NULL/E_GENERIC_OOM  out of memory
regex_ct regex_compile(regex_compiler_const_ct c, const char *text, size_t len);

/// Free regular expression.
///
/// \param re       regex
void regex_free(regex_ct re);

/// Execute a regular expression.
///
/// \param re       regex
/// \param text     text
/// \param len      \p text length
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int regex_exec(regex_const_ct re, const char *text, size_t len);


#endif
