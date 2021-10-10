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

#include <ytil/gen/regex.h>
#include <ytil/parser/regex.h>
#include <ytil/def/magic.h>
#include <stdlib.h>


#define MAGIC_COMPILER  define_magic("RXC")   ///< regex compilermagic
#define MAGIC_REGEX     define_magic("RGX")   ///< regex magic


/// regex compiler
typedef struct regex_compiler
{
    DEBUG_MAGIC

    parser_ct   p;  ///< regex parser
} regex_compiler_st;

/// regex
typedef struct regex
{
    DEBUG_MAGIC

    parser_ct   p;  ///< parser
} regex_st;


/// regex error type definition
ERROR_DEFINE_LIST(REGEX,
    ERROR_INFO(E_REGEX_FOO, "foo")
);

/// default error type for regex module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_REGEX


regex_compiler_ct regex_compiler_new(void)
{
    regex_compiler_ct c;

    if(!(c = calloc(1, sizeof(regex_compiler_st))))
        return error_wrap_last_errno(calloc), NULL;

    init_magic_n(c, MAGIC_COMPILER);

    if(!(c->p = parser_regex()))
        return error_wrap(), free(c), NULL;

    return c;
}

void regex_compiler_free(regex_compiler_ct c)
{
    assert_magic_n(c, MAGIC_COMPILER);

    parser_free(c->p);
    free(c);
}

regex_ct regex_compile(regex_compiler_const_ct c, const char *text, size_t len)
{
    regex_ct re;

    assert_magic_n(c, MAGIC_COMPILER);

    if(!(re = calloc(1, sizeof(regex_st))))
        return error_wrap_last_errno(calloc), NULL;

    init_magic_n(re, MAGIC_REGEX);

    /// \todo
    //if(!(re->p = parser_fail()))
    //    return error_wrap(), free(re), NULL;

    return re;
}

void regex_free(regex_ct re)
{
    assert_magic_n(re, MAGIC_REGEX);

    parser_free(re->p);
    free(re);
}

int regex_exec(regex_const_ct re, const char *text, size_t len)
{
    assert_magic_n(re, MAGIC_REGEX);

    /// \todo
    // parser_parse(re->p, text, len, NULL);

    return 0;
}
