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

#ifndef YTIL_PARSER_CHAR_H_INCLUDED
#define YTIL_PARSER_CHAR_H_INCLUDED

#include <ytil/parser/parser.h>
#include <ytil/ext/ctype.h>


/// New parser matching an arbitrary character.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_any(void);

/// New parser matching a specific character.
///
/// \param c    character to parse
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_char(char c);

/// New parser matching an arbitrary character which is not a specific character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_char(c)), parser_any())
///
/// \param c    character to parse
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_char(char c);

/// New parser matching a character matching a predicate.
///
/// \param pred     predicate
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_pred(ctype_pred_cb pred);

/// New parser matching a character not matching a predicate.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_pred(pred)), parser_any())
///
/// \param pred     predicate
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_pred(ctype_pred_cb pred);

/// New parser matching a decimal digit character.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_digit(void);

/// New parser matching an arbitrary character which is not a decimal digit character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_digit()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_digit(void);

/// New parser matching a binary digit character.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_bdigit(void);

/// New parser matching an arbitrary character which is not a binary digit character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_bdigit()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_bdigit(void);

/// New parser matching an octal digit character.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_odigit(void);

/// New parser matching an arbitrary character which is not an octal digit character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_odigit()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_odigit(void);

/// New parser matching a hexadecimal digit character.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_xdigit(void);

/// New parser matching an arbitrary character which is not a hexadecimal digit character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_xdigit()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_xdigit(void);

/// New parser matching a lowercase hexadecimal digit character.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_lxdigit(void);

/// New parser matching an arbitrary character which is not a lowercase hexadecimal digit character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_lxdigit()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lxdigit(void);

/// New parser matching an uppercase hexadecimal digit character.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_uxdigit(void);

/// New parser matching an arbitrary character which is not an uppercase hexadecimal digit character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_uxdigit()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_uxdigit(void);

/// New parser matching a lowercase character.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_lower(void);

/// New parser matching an arbitrary character which is not a lowercase character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_lower()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_lower(void);

/// New parser matching an uppercase character.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_upper(void);

/// New parser matching an arbitrary character which is not an uppercase character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_upper()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_upper(void);

/// New parser matching an alphanumeric character.
///
/// Parses alphabetic and numeric characters as defined by isalnum().
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_alnum(void);

/// New parser matching an arbitrary character which is not an alphanumeric character.
///
/// Parses any character which is not an alphabetic
/// or numeric character as defined by isalnum().
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_alnum()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_alnum(void);

/// New parser matching an alphabetic character.
///
/// Parses alphabetic characters as defined by isalpha().
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_alpha(void);

/// New parser matching an arbitrary character which is not an alphabetic character.
///
/// Parses any character which is not an alphabetic
/// character as defined by isalpha().
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_alpha()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_alpha(void);

/// New parser matching a 7bit ASCII character.
///
/// Parses 7bit ASCII characters as defined by isascii().
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_ascii(void);

/// New parser matching an arbitrary character which is not a 7bit ASCII character.
///
/// Parses any character which is not a 7bit ASCII
/// character as defined by isascii().
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_ascii()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_ascii(void);

/// New parser matching a blank character.
///
/// Parses space or tab characters as defined by isblank().
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_blank(void);

/// New parser matching an arbitrary character which is not a blank character.
///
/// Parses any character which is not a space or tab
/// character as defined by isblank().
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_blank()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_blank(void);

/// New parser matching a control character.
///
/// Parses control characters as defined by iscntrl().
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_cntrl(void);

/// New parser matching an arbitrary character which is not a control character.
///
/// Parses any character which is not a control
/// character as defined by iscntrl().
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_cntrl()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_cntrl(void);

/// New parser matching a graphical character.
///
/// Parses printable characters except space as defined by isgraph().
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_graph(void);

/// New parser matching an arbitrary character which is not a graphical character.
///
/// Parses any character which is not a printable
/// character except space as defined by isgraph().
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_graph()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_graph(void);

/// New parser matching a printable character.
///
/// Parses printable characters including space as defined by isprint().
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_print(void);

/// New parser matching an arbitrary character which is not a printable character.
///
/// Parses any character which is not a printable character
/// including space as defined by isprint().
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_print()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_print(void);

/// New parser matching a punctuation character.
///
/// Parses printable characters except space or
/// alphanumeric characters as defined by ispunct().
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_punct(void);

/// New parser matching an arbitrary character which is not a punctuation character.
///
/// Parses any character which is not a printable character
/// except space or alphanumeric character as defined by ispunct().
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_punct()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_punct(void);

/// New parser matching a whitespace character.
///
/// Parses whitespace characters as defined by isspace().
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_whitespace(void);

/// New parser matching an arbitrary character which is not a whitespace character.
///
/// Parses any character which is not a whitespace
/// character as defined by isspace().
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_whitespace()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_whitespace(void);

/// New parser matching a sign character.
///
/// Parses plus and minus characters.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_sign(void);

/// New parser matching an arbitrary character which is not a sign character.
///
/// Parses any character which is not a plus and minus character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_sign()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_sign(void);

/// New parser matching a word character.
///
/// Parses underscore and alphanumeric characters.
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_word(void);

/// New parser matching an arbitrary character which is not a word character.
///
/// Parses any character which is not an underscore and alphanumeric character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_word()), parser_any())
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_word(void);

/// New parser matching a specific acceptable character.
///
/// \param accept   acceptable characters
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_accept(const char *accept);

/// New parser matching an arbitrary acceptable character.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_accept(reject)), parser_any())
///
/// \param reject   unacceptable characters
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_reject(const char *reject);

/// New parser matching a character from a character range.
///
/// \param start    first acceptable character
/// \param end      last acceptable character
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_range(char start, char end);

/// New parser matching a character from outside a character range.
///
/// \par Equivalent
///     parser_and(2, parser_not(parser_range(start, end)), parser_any())
///
/// \param start    first unacceptable character
/// \param end      last unacceptable character
///
/// \returns                    parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_not_range(char start, char end);


#endif
