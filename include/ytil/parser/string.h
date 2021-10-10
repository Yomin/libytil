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

#ifndef YTIL_PARSER_STRING_H_INCLUDED
#define YTIL_PARSER_STRING_H_INCLUDED

#include <ytil/parser/parser.h>
#include <stdarg.h>


/*typedef struct parser_string
{
    const char  *data;
    size_t      len;
} parser_string_st;*/


/// Create new parser which parses a string.
///
/// \param str  string
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_string(const char *str);

/// Create new parser which parses one of a list of accepted strings.
///
/// \param n    number of accepted strings
/// \param ...  variadic list of const char*
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_string_accept(size_t n, ...);

/// Create new parser which parses one of a list of accepted strings.
///
/// \param n        number of accepted strings
/// \param strings  variadic list of const char*
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_string_accept_v(size_t n, va_list strings);


#endif
