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
#include <ytil/ext/ctype.h>
#include <stdarg.h>


typedef struct parser_match
{
    const void  *data;
    size_t      size;
} parser_match_st;

int parser_stack_push_match(parser_stack_ct stack, const void *data, size_t size);


/// New parser which matches a string.
///
/// \par Equivalent
///     parser_seq(n, parser_char(str[0]), parser_char(str[1]), ..., parser_char(str[n-1]))
///
/// \param str  string
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_string(const char *str);

/// New parser which matches as many characters as possible which satisfy a predicate.
///
/// \par Equivalent
///     parser_many(parser_pred(pred))
///
/// \param pred     predicate
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_while(ctype_pred_cb pred);

/// New parser which matches at least \p n characters which satisfy a predicate.
///
/// \par Equivalent
///     parser_min(n, parser_pred(pred))
///
/// \param n        minimum number of matches
/// \param pred     predicate
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_while_min(size_t n, ctype_pred_cb pred);

/// New parser which matches as many characters as possible which do not satisfy a predicate.
///
/// \par Equivalent
///     parser_many(parser_not_pred(pred))
///
/// \param pred     predicate
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_until(ctype_pred_cb pred);

/// New parser which matches as many characters as possible which do not satisfy a predicate.
///
/// \par Equivalent
///     parser_many(parser_not_pred(pred))
///
/// \param n        minimum number of matches
/// \param pred     predicate
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_until_min(size_t n, ctype_pred_cb pred);

/// New parser which matches at least \p n decimal digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_digit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_digits(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not decimal digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_digit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_digits(size_t n);

/// New parser which matches at least \p n binary digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_bdigit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_bdigits(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not binary digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_bdigit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_bdigits(size_t n);

/// New parser which matches at least \p n octal digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_odigit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_odigits(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not octal digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_odigit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_odigits(size_t n);

/// New parser which matches at least \p n hexadecimal digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_xdigit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_xdigits(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not hexadecimal digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_xdigit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_xdigits(size_t n);

/// New parser which matches at least \p n lowercase hexadecimal digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_lxdigit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_lxdigits(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not lowercase hexadecimal digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_lxdigit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lxdigits(size_t n);

/// New parser which matches at least \p n uppercase hexadecimal digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_uxdigit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_uxdigits(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not uppercase hexadecimal digit characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_uxdigit())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_uxdigits(size_t n);

/// New parser which matches at least \p n lowercase characters.
///
/// \par Equivalent
///     parser_min(n, parser_lower())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_lowers(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not lowercase characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_lower())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lowers(size_t n);

/// New parser which matches at least \p n uppercase characters.
///
/// \par Equivalent
///     parser_min(n, parser_upper())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_uppers(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not uppercase characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_upper())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_uppers(size_t n);

/// New parser which matches at least \p n alphanumeric characters.
///
/// \par Equivalent
///     parser_min(n, parser_alnum())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_alnums(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not alphanumeric characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_alnum())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_alnums(size_t n);

/// New parser which matches at least \p n alphabetic characters.
///
/// \par Equivalent
///     parser_min(n, parser_alpha())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_alphas(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not alphabetic characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_alpha())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_alphas(size_t n);

/// New parser which matches at least \p n 7bit ASCII characters.
///
/// \par Equivalent
///     parser_min(n, parser_ascii())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_asciis(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not 7bit ASCII characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_ascii())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_asciis(size_t n);

/// New parser which matches at least \p n blank characters.
///
/// \par Equivalent
///     parser_min(n, parser_blank())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_blanks(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not blank characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_blank())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_blanks(size_t n);

/// New parser which matches at least \p n control characters.
///
/// \par Equivalent
///     parser_min(n, parser_cntrl())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_cntrls(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not control characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_cntrl())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_cntrls(size_t n);

/// New parser which matches at least \p n graphical characters.
///
/// \par Equivalent
///     parser_min(n, parser_graph())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_graphs(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not graphical characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_graph())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_graphs(size_t n);

/// New parser which matches at least \p n printable characters.
///
/// \par Equivalent
///     parser_min(n, parser_print())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_prints(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not printable characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_print())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_prints(size_t n);

/// New parser which matches at least \p n punctuation characters.
///
/// \par Equivalent
///     parser_min(n, parser_punct())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_puncts(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not punctuation characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_punct())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_puncts(size_t n);

/// New parser which matches at least \p n whitespace characters.
///
/// \par Equivalent
///     parser_min(n, parser_whitespace())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_whitespaces(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not whitespace characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_whitespace())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_whitespaces(size_t n);

/// New parser which matches at least \p n sign characters.
///
/// \par Equivalent
///     parser_min(n, parser_sign())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_signs(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not sign characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_sign())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_signs(size_t n);

/// New parser which matches at least \p n word characters.
///
/// \par Equivalent
///     parser_min(n, parser_word())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_words(size_t n);

/// New parser which matches at least \p n arbitrary characters
/// which are not word characters.
///
/// \par Equivalent
///     parser_min(n, parser_not_word())
///
/// \param n    minimum number of matches
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_words(size_t n);

/// New parser which matches one of a list of accepted strings.
///
/// \par Equivalent
///     parser_or(n, parser_string(s1), parser_string(s2), ..., parser_string(sn))
///
/// \param n    number of accepted strings
/// \param ...  variadic list of const char*
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_string_accept(size_t n, ...);

/// New parser which matches one of a list of accepted strings.
///
/// \param n        number of accepted strings
/// \param strings  variadic list of const char*
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_string_accept_v(size_t n, va_list strings);


#endif
