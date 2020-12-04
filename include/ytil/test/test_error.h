/*
 * Copyright (c) 2019-2020 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_TEST_TEST_ERROR_H_INCLUDED
#define YTIL_TEST_TEST_ERROR_H_INCLUDED

#include <ytil/test/test.h>
#include <ytil/gen/error.h>


/// Test error at depth to match type and code.
///
/// \param depth    error depth beginning with last error
/// \param type     error type name
/// \param code     error code
#define test_error(depth, type, code) do {                                    \
    test_begin();                                                             \
                                                                              \
    size_t _depth               = (depth);                                    \
    const error_type_st *type1  = error_type(_depth);                         \
    const error_type_st *type2  = ERROR_TYPE(type);                           \
    int code1                   = error_code(_depth);                         \
    int code2                   = (code);                                     \
                                                                              \
    if(type1 != type2)                                                        \
    {                                                                         \
        test_abort_fail_b("ERROR type test failed\n%s == %s",                 \
            error_type_name(type1), error_type_name(type2));                  \
    }                                                                         \
    else if(code1 != code2)                                                   \
    {                                                                         \
        test_abort_fail_b("%s test failed\n%s (%d) == %s (%d)",               \
            error_type_name(type1), error_name(_depth), code1, #code, code2); \
    }                                                                         \
                                                                              \
    test_end();                                                               \
} while(0)

/// Test error on stack at level to match type and code.
///
/// \param level    error stack level beginning with first error
/// \param type     error type name
/// \param code     error code
#define test_stack_error(level, type, code) do {                                        \
    test_begin();                                                                       \
                                                                                        \
    size_t _level               = (level);                                              \
    const error_type_st *type1  = error_stack_get_type(_level);                         \
    const error_type_st *type2  = ERROR_TYPE(type);                                     \
    int code1                   = error_stack_get_code(_level);                         \
    int code2                   = (code);                                               \
                                                                                        \
    if(type1 != type2)                                                                  \
    {                                                                                   \
        test_abort_fail_b("ERROR type test failed\n%s == %s",                           \
            error_type_name(type1), error_type_name(type2));                            \
    }                                                                                   \
    else if(code1 != code2)                                                             \
    {                                                                                   \
        test_abort_fail_b("%s test failed\n%s (%d) == %s (%d)",                         \
            error_type_name(type1), error_stack_get_name(_level), code1, #code, code2); \
    }                                                                                   \
                                                                                        \
    test_end();                                                                         \
} while(0)


#endif // ifndef YTIL_TEST_TEST_ERROR_H_INCLUDED
