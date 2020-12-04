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

#ifndef YTIL_TEST_TEST_H_INCLUDED
#define YTIL_TEST_TEST_H_INCLUDED

#include <ytil/test/case.h>


/// Update position for next test.
///
///
#define test_begin() \
    test_case_begin_test(__FILE__, __LINE__)

/// Update position after test.
///
///
#define test_end() \
    test_case_end_test(__FILE__, __LINE__)

/// Trace function call.
///
/// \param call     call to trace
#define test_trace(call) do {                       \
    test_case_push_call(__FILE__, __LINE__, #call); \
    call;                                           \
    test_case_pop_call();                           \
} while(0)

/// Trace function call and get rc.
///
/// \param call     call to trace
#define test_trace_rc(call) __extension__ ({        \
    test_case_push_call(__FILE__, __LINE__, #call); \
    __auto_type rc = (call);                        \
    test_case_pop_call();                           \
    rc;                                             \
})

/// Abort test case.
///
///
#define test_abort() \
    test_case_abort()

/// Abort test case on missing dependency.
///
/// \param msg      missing description
/// \param ...      msg arguments
#define test_abort_missing(msg, ...) \
    test_case_abort_missing(__FILE__, __LINE__, (msg) __VA_OPT__(,) __VA_ARGS__)

/// Abort test case on failing test.
///
/// \param msg      failure reason
/// \param ...      msg arguments
#define test_abort_fail(msg, ...) \
    test_case_abort_fail(__FILE__, __LINE__, (msg) __VA_OPT__(,) __VA_ARGS__)

/// Abort test case on failing test and append error backtrace.
///
/// \param msg      failure reason
/// \param ...      msg arguments
#define test_abort_fail_b(msg, ...) \
    test_case_abort_fail_b(__FILE__, __LINE__, (msg) __VA_OPT__(,) __VA_ARGS__)


/// Add test case message.
///
/// \param type     message type
/// \param msg      message
/// \param ...      message arguments
#define test_msg(type, msg, ...) \
    test_case_add_msg(__FILE__, __LINE__, (type), (msg) __VA_OPT__(,) __VA_ARGS__)

/// Add test case info message.
///
/// \param msg      message
/// \param ...      message arguments
#define test_msg_info(msg, ...) \
    test_msg(TEST_MSG_INFO, (msg) __VA_OPT__(,) __VA_ARGS__)

/// Add test case warn message.
///
/// \param msg      message
/// \param ...      message arguments
#define test_msg_warn(msg, ...) \
    test_msg(TEST_MSG_WARN, (msg) __VA_OPT__(,) __VA_ARGS__)

/// Add test case missing message.
///
/// \param msg      message
/// \param ...      message arguments
#define test_msg_missing(msg, ...) \
    test_msg(TEST_MSG_MISSING, (msg) __VA_OPT__(,) __VA_ARGS__)

/// Add test case fail message.
///
/// \param msg      message
/// \param ...      message arguments
#define test_msg_fail(msg, ...) \
    test_msg(TEST_MSG_FAIL, (msg) __VA_OPT__(,) __VA_ARGS__)

/// Add test case message if check passes.
///
/// \param expr     expression to check
/// \param type     message type
/// \param msg      message
/// \param ...      message arguments
#define test_check(expr, type, msg, ...) do {              \
    test_begin();                                          \
                                                           \
    if((expr))                                             \
        test_msg((type), (msg) __VA_OPT__(,) __VA_ARGS__); \
                                                           \
    test_end();                                            \
} while(0)

/// Add test case info message if check passes.
///
/// \param expr     expression to check
/// \param msg      message
/// \param ...      message arguments
#define test_check_info(expr, msg, ...) \
    test_check((expr), TEST_MSG_INFO, (msg) __VA_OPT__(,) __VA_ARGS__)

/// Add test case warn message if check passes.
///
/// \param expr     expression to check
/// \param msg      message
/// \param ...      message arguments
#define test_check_warn(expr, msg, ...) \
    test_check((expr), TEST_MSG_WARN, (msg) __VA_OPT__(,) __VA_ARGS__)

/// Add test case missing message if check passes.
///
/// \param expr     expression to check
/// \param msg      message
/// \param ...      message arguments
#define test_check_missing(expr, msg, ...) \
    test_check((expr), TEST_MSG_MISSING, (msg) __VA_OPT__(,) __VA_ARGS__)

/// Add test case fail message if check passes.
///
/// \param expr     expression to check
/// \param msg      message
/// \param ...      message arguments
#define test_check_fail(expr, msg, ...) \
    test_check((expr), TEST_MSG_FAIL, (msg) __VA_OPT__(,) __VA_ARGS__)


/// Test nothing, just capsulate expr to update position.
///
/// \param expr     expression to test
#define test_void(expr) do { \
    test_begin();            \
                             \
    expr;                    \
                             \
    test_end();              \
} while(0)


#include <ytil/test/test_error.h>
#include <ytil/test/test_rc.h>
#include <ytil/test/test_value.h>


#endif // ifndef YTIL_TEST_TEST_H_INCLUDED
