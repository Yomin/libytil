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

#ifndef YTIL_TEST_CASE_H_INCLUDED
#define YTIL_TEST_CASE_H_INCLUDED

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>


/// test end type
typedef enum test_end_type
{
    TEST_END_NORMAL,    ///< normal end
    TEST_END_EXIT,      ///< test case must exit with specific rc
    TEST_END_SIGNAL,    ///< test case must abort with specific signal
    TEST_END_TYPES,     ///< number of end types
} test_end_id;

/// test message type
typedef enum test_msg_type
{
    TEST_MSG_INFO,      ///< info message
    TEST_MSG_WARN,      ///< warn message
    TEST_MSG_MISSING,   ///< missing message
    TEST_MSG_FAIL,      ///< fail message
    TEST_MSG_ERROR,     ///< error message
    TEST_MSG_TYPES,     ///< number of message types
} test_msg_id;

/// test status type
typedef enum test_status_type
{
    TEST_STATUS_SETUP,      ///< test case executes setup
    TEST_STATUS_EXEC,       ///< test case executes tests
    TEST_STATUS_TEARDOWN,   ///< test case executes teardown
    TEST_STATUS_CLEANUP,    ///< test case executes cleanup
    TEST_STATUS_DONE,       ///< test case is done
    TEST_STATUS_TYPES,      ///< number of status types
} test_status_id;

/// test result type
typedef enum test_result_type
{
    TEST_RESULT_PASS,       ///< test case passed
    TEST_RESULT_SKIP,       ///< test case was skipped
    TEST_RESULT_TIMEOUT,    ///< test case timed out
    TEST_RESULT_WARN,       ///< test case had warning
    TEST_RESULT_MISSING,    ///< test case had missing dependencies
    TEST_RESULT_FAIL,       ///< test case failed
    TEST_RESULT_ERROR,      ///< test case had error
    TEST_RESULT_TYPES,      ///< number of result types
} test_result_id;

/// test config
typedef struct test_config
{
    bool    clean;      ///< clean up after test case
    bool    dump;       ///< do not disable core dumps
    bool    fork;       ///< fork to execute test case
    bool    skip;       ///< skip test case with non-normal end type
    bool    stdio;      ///< do not suppress test case stdio
    size_t  timeout;    ///< test case timeout in seconds
} test_config_st;

/// test position
typedef struct test_pos
{
    char    *file;      ///< file, may be NULL
    size_t  line;       ///< line in file
    bool    after;      ///< position is after line
} test_pos_st;

/// test message line
typedef struct test_line
{
    int     level;      ///< level
    char    *msg;       ///< msg, may be NULL
} test_line_st;

/// test call
typedef struct test_call
{
    test_pos_st pos;    ///< call position
    char        *call;  ///< call text, may be NULL
} test_call_st;

/// test message
typedef struct test_msg
{
    test_pos_st     pos;        ///< message origin
    test_msg_id     type;       ///< message type
    test_call_st    *call;      ///< call stack, may be NULL
    size_t          calls;      ///< number of calls
    test_line_st    *line;      ///< message lines
    size_t          lines;      ///< number of lines
} test_msg_st;

/// test message fold callback
///
/// \param msg      message
/// \param ctx      callback context
typedef void (*test_case_msg_cb)(const test_msg_st *msg, void *ctx);


/// test case callback
///
///
typedef void (*test_case_cb)(void);

/// test case
typedef struct test_case
{
    const char      *name;      ///< test case name
    test_case_cb    cb;         ///< test case callback
    test_case_cb    setup;      ///< setup callback
    test_case_cb    teardown;   ///< teardown callback
    test_case_cb    cleanup;    ///< cleanup callback
    test_end_id     end_type;   ///< test case end type
    int             end_value;  ///< test case end value (rc or signal)
} test_case_st;


/// Create test case setup.
///
/// Setup everything needed for a test case.
/// Setups are executed in the worker process.
///
/// \param name     setup name
#define TEST_SETUP(name) \
    static void test_case_setup_ ## name(void)

/// Create parameterized test case setup.
///
/// Setup everything needed for a test case.
/// Setups are executed in the worker process.
///
/// \param name     setup name
/// \param ...      parameters
#define TEST_PSETUP(name, ...) \
    static void test_case_psetup_ ## name(__VA_ARGS__)

/// Create test case setup delegate.
///
/// \param name     setup name
/// \param setup    parameterized setup callback
/// \param ...      setup parameters
#define TEST_SETUP_DELEGATE(name, setup, ...)    \
    TEST_SETUP(name)                             \
    {                                            \
        test_case_psetup_ ## setup(__VA_ARGS__); \
    }

/// Create test case teardown.
///
/// Restore the application state to the state before setup.
/// Teardowns are only executed if the setup completed successfully.
/// Teardowns are not executed if the worker aborted during the test case.
/// Teardowns are executed in the worker process.
///
/// \param name     teardown name
#define TEST_TEARDOWN(name) \
    static void test_case_teardown_ ## name(void)

/// Create test case cleanup.
///
/// Restore the external state (files, databases, etc) to the state before setup.
/// Cleanups are always executed after teardown in the control process.
///
/// \param name     cleanup name
#define TEST_CLEANUP(name) \
    static void test_case_cleanup_ ## name(void)

#define test_case_setup_no_setup        NULL    ///< allow no_setup as setup name
#define test_case_teardown_no_teardown  NULL    ///< allow no_teardown as teardown name
#define test_case_cleanup_no_cleanup    NULL    ///< allow no_cleanup as cleanup name

/// Create test case.
///
/// \param _name        name
/// \param _end_type    end type
/// \param _end_value   end value
/// \param _setup       setup callback
/// \param _teardown    teardown callback
/// \param _cleanup     cleanup callback
#define TEST_CASE_FULL(_name, _end_type, _end_value, _setup, _teardown, _cleanup) \
    static void test_case_cb_ ## _name(void);                                     \
                                                                                  \
    static const test_case_st test_case_ ## _name =                               \
    {                                                                             \
        .name       = #_name,                                                     \
        .cb         = test_case_cb_ ## _name,                                     \
        .setup      = test_case_setup_ ## _setup,                                 \
        .teardown   = test_case_teardown_ ## _teardown,                           \
        .cleanup    = test_case_cleanup_ ## _cleanup,                             \
        .end_type   = _end_type,                                                  \
        .end_value  = _end_value                                                  \
    };                                                                            \
                                                                                  \
    static void test_case_cb_ ## _name(void)

/// Create parameterized test case.
///
/// \param name         name
/// \param end_type     end type
/// \param end_value    end value
/// \param setup        setup callback
/// \param teardown     teardown callback
/// \param cleanup      cleanup callback
/// \param ...          setup parameters
#define TEST_CASE_PFULL(name, end_type, end_value, setup, teardown, cleanup, ...) \
    TEST_SETUP_DELEGATE(name, setup, __VA_ARGS__)                                 \
    TEST_CASE_FULL(name, end_type, end_value, name, teardown, cleanup)


/// Create normal test case.
///
/// \param name     test case name
#define TEST_CASE(name) \
    TEST_CASE_FULL(name, TEST_END_NORMAL, 0, no_setup, no_teardown, no_cleanup)

/// Create normal test case with fixture.
///
/// \param name     test case name
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
#define TEST_CASE_FIX(name, setup, teardown) \
    TEST_CASE_FULL(name, TEST_END_NORMAL, 0, setup, teardown, no_cleanup)

/// Create normal test case with parameterized fixture.
///
/// \param name     test case name
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param ...      setup parameters
#define TEST_CASE_PFIX(name, setup, teardown, ...) \
    TEST_CASE_PFULL(name, TEST_END_NORMAL, 0, setup, teardown, no_cleanup, __VA_ARGS__)

/// Create normal test case with cleanup fixture.
///
/// \param name     test case name
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param cleanup  cleanup function to call after teardown
#define TEST_CASE_CLEAN(name, setup, teardown, cleanup) \
    TEST_CASE_FULL(name, TEST_END_NORMAL, 0, setup, teardown, cleanup)

/// Create normal test case with parameterized cleanup fixture.
///
/// \param name     test case name
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param cleanup  cleanup function to call after teardown
/// \param ...      setup parameters
#define TEST_CASE_PCLEAN(name, setup, teardown, cleanup, ...) \
    TEST_CASE_PFULL(name, TEST_END_NORMAL, 0, setup, teardown, cleanup, __VA_ARGS__)


/// Create exit expecting test case.
///
/// \param name     test case name
/// \param rc       expected exit code
#define TEST_CASE_EXIT(name, rc) \
    TEST_CASE_FULL(name, TEST_END_EXIT, (rc), no_setup, no_teardown, no_cleanup)

/// Create exit expecting test case with fixture.
///
/// \param name     test case name
/// \param rc       exit code
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
#define TEST_CASE_FIX_EXIT(name, rc, setup, teardown) \
    TEST_CASE_FULL(name, TEST_END_EXIT, (rc), setup, teardown, no_cleanup)

/// Create exit expecting test case with parameterized fixture.
///
/// \param name     test case name
/// \param rc       exit code
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param ...      setup parameters
#define TEST_CASE_PFIX_EXIT(name, rc, setup, teardown, ...) \
    TEST_CASE_PFULL(name, TEST_END_EXIT, (rc), setup, teardown, no_cleanup, __VA_ARGS__)

/// Create exit expecting test case with cleanup fixture.
///
/// \param name     test case name
/// \param rc       exit code
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param cleanup  cleanup function to call after teardown
#define TEST_CASE_CLEAN_EXIT(name, rc, setup, teardown, cleanup) \
    TEST_CASE_FULL(name, TEST_END_EXIT, (rc), setup, teardown, cleanup)

/// Create exit expecting test case with parameterized cleanup fixture.
///
/// \param name     test case name
/// \param rc       exit code
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param cleanup  cleanup function to call after teardown
/// \param ...      setup parameters
#define TEST_CASE_PCLEAN_EXIT(name, rc, setup, teardown, cleanup, ...) \
    TEST_CASE_PFULL(name, TEST_END_EXIT, (rc), setup, teardown, cleanup, __VA_ARGS__)


/// Create signal expecting test case.
///
/// \param name     test case name
/// \param signal   expected signal
#define TEST_CASE_SIGNAL(name, signal) \
    TEST_CASE_FULL(name, TEST_END_SIGNAL, (signal), no_setup, no_teardown, no_cleanup)

/// Create signal expecting test case with fixture.
///
/// \param name     test case name
/// \param signal   signal number
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
#define TEST_CASE_FIX_SIGNAL(name, signal, setup, teardown) \
    TEST_CASE_FULL(name, TEST_END_SIGNAL, (signal), setup, teardown, no_cleanup)

/// Create signal expecting test case with parameterized fixture.
///
/// \param name     test case name
/// \param signal   signal number
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param ...      setup parameters
#define TEST_CASE_PFIX_SIGNAL(name, signal, setup, teardown, ...) \
    TEST_CASE_PFULL(name, TEST_END_SIGNAL, (signal), setup, teardown, no_cleanup, __VA_ARGS__)

/// Create signal expecting test case with cleanup fixture.
///
/// \param name     test case name
/// \param signal   signal number
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param cleanup  cleanup function to call after teardown
#define TEST_CASE_CLEAN_SIGNAL(name, signal, setup, teardown, cleanup) \
    TEST_CASE_FULL(name, TEST_END_SIGNAL, (signal), setup, teardown, cleanup)

/// Create signal expecting test case with parameterized cleanup fixture.
///
/// \param name     test case name
/// \param signal   signal number
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param cleanup  cleanup function to call after teardown
/// \param ...      setup parameters
#define TEST_CASE_PCLEAN_SIGNAL(name, signal, setup, teardown, cleanup, ...) \
    TEST_CASE_PFULL(name, TEST_END_SIGNAL, (signal), setup, teardown, cleanup, __VA_ARGS__)


/// Create SIGABRT expecting test case.
///
/// \param name     test case name
#define TEST_CASE_ABORT(name) \
    TEST_CASE_FULL(name, TEST_END_SIGNAL, SIGABRT, no_setup, no_teardown, no_cleanup)

/// Create SIGABRT expecting test case with fixture.
///
/// \param name     test case name
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
#define TEST_CASE_FIX_ABORT(name, setup, teardown) \
    TEST_CASE_FULL(name, TEST_END_SIGNAL, SIGABRT, setup, teardown, no_cleanup)

/// Create SIGABRT expecting test case with parameterized fixture.
///
/// \param name     test case name
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param ...      setup parameters
#define TEST_CASE_PFIX_ABORT(name, setup, teardown, ...) \
    TEST_CASE_PFULL(name, TEST_END_SIGNAL, SIGABRT, setup, teardown, no_cleanup, __VA_ARGS__)

/// Create SIGABRT expecting test case with cleanup fixture.
///
/// \param name     test case name
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param cleanup  cleanup function to call after teardown
#define TEST_CASE_CLEAN_ABORT(name, setup, teardown, cleanup) \
    TEST_CASE_FULL(name, TEST_END_SIGNAL, SIGABRT, setup, teardown, cleanup)

/// Create SIGABRT expecting test case with parameterized cleanup fixture.
///
/// \param name     test case name
/// \param setup    setup function to call before test case
/// \param teardown teardown function to call after test case
/// \param cleanup  cleanup function to call after teardown
/// \param ...      setup parameters
#define TEST_CASE_PCLEAN_ABORT(name, setup, teardown, cleanup, ...) \
    TEST_CASE_PFULL(name, TEST_END_SIGNAL, SIGABRT, setup, teardown, cleanup, __VA_ARGS__)


/// Get test case.
///
/// \param name     test case name
///
/// \returns        pointer to test case
#define test_case(name) \
    &test_case_ ## name

/// NOP test case which is ignored by test_run_cases().
#define TEST_CASE_NOP ((const test_case_st *)1)

/// Unix only test case.
#define test_case_unix(name) \
    UNIX_WINDOWS(test_case(name), TEST_CASE_NOP)

/// Windows only test case.
#define test_case_windows(name) \
    UNIX_WINDOWS(TEST_CASE_NOP, test_case(name))


/// Begin test. Used from within test cases.
///
/// \param file     file containing test
/// \param line     line in file
void test_case_begin_test(const char *file, size_t line);

/// End test. Used from within test cases.
///
/// \param file     file containing test
/// \param line     line in file
void test_case_end_test(const char *file, size_t line);

/// Abort test case. Used from within test cases.
///
///
__attribute__((noreturn))
void test_case_abort(void);

/// Abort test case with 'missing' result. Used from within test cases.
///
/// \param file     file
/// \param line     line in file
/// \param msg      abort reason
/// \param ...      msg arguments
__attribute__((format(gnu_printf, 3, 4), noreturn))
void test_case_abort_missing(const char *file, size_t line, const char *msg, ...);

/// Abort test case with 'fail' result. Used from within test cases.
///
/// \param file         file
/// \param line         line in file
/// \param msg          abort reason
/// \param ...          msg arguments
__attribute__((format(gnu_printf, 3, 4), noreturn))
void test_case_abort_fail(const char *file, size_t line, const char *msg, ...);

/// Abort test case with 'fail' result and append error backtrace. Used from within test cases.
///
/// \param file         file
/// \param line         line in file
/// \param msg          abort reason
/// \param ...          msg arguments
__attribute__((format(gnu_printf, 3, 4), noreturn))
void test_case_abort_fail_b(const char *file, size_t line, const char *msg, ...);

/// Push call on call stack. Used from within test cases.
///
/// \param file     position file
/// \param line     position line
/// \param call     call
void test_case_push_call(const char *file, size_t line, const char *call);

/// Pop call from call stack. Used from within test cases.
///
///
void test_case_pop_call(void);

/// Add message. Used from within test cases.
///
/// \param file     file
/// \param line     line in file
/// \param type     message type
/// \param msg      message
/// \param ...      msg arguments
__attribute__((format(gnu_printf, 4, 5)))
void test_case_add_msg(const char *file, size_t line, test_msg_id type, const char *msg, ...);

/// Add message. Used from within test cases.
///
/// \param file     file
/// \param line     line in file
/// \param type     message type
/// \param msg      message
/// \param ap       msg arguments
__attribute__((format(gnu_printf, 4, 0)))
void test_case_add_msg_v(const char *file, size_t line, test_msg_id type, const char *msg, va_list ap);

/// Append line to last message. Used from within test cases.
///
/// \param level    message level
/// \param msg      message, may be NULL for blank line
/// \param ...      msg arguments
__attribute__((format(gnu_printf, 2, 3)))
void test_case_append_msg(int level, const char *msg, ...);

/// Append line to last message. Used from within test cases.
///
/// \param level    message level
/// \param msg      message, may be NULL for blank line
/// \param ap       msg arguments
__attribute__((format(gnu_printf, 2, 0)))
void test_case_append_msg_v(int level, const char *msg, va_list ap);

/// Append error backtrace to last message. Used from within test cases.
///
/// \param level    backtrace level
void test_case_append_backtrace(int level);

/// Fold over all test case messages.
///
/// \param fold     fold callback
/// \param ctx      callback context
void test_case_fold_msg(test_case_msg_cb fold, const void *ctx);

/// Run test case.
///
/// \param tcase    test case
/// \param config   test config
///
/// \retval 0                   success
/// \retval >0                  worker process, cleanup and exit
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
int test_case_run(const test_case_st *tcase, const test_config_st *config);

/// Get test case name.
///
/// \returns    test case name
const char *test_case_name(void);

/// Get test case result.
///
/// \returns    test result
test_result_id test_case_result(void);

/// Free test case.
///
///
void test_case_free(void);


#endif // ifndef YTIL_TEST_CASE_H_INCLUDED
