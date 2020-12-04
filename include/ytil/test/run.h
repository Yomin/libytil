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

#ifndef YTIL_TEST_RUN_H_INCLUDED
#define YTIL_TEST_RUN_H_INCLUDED

#include <ytil/test/case.h>
#include <ytil/gen/error.h>


/// test error
typedef enum test_error
{
    E_TEST_DISABLED,    ///< disabled test suite/case
    E_TEST_MISSING,     ///< test suite is missing something to run
    E_TEST_NOFORK,      ///< fork not available
    E_TEST_STOP,        ///< a test case had stop result
    E_TEST_USAGE,       ///< invalid cmdline usage
} test_error_id;

/// test error type declaration
ERROR_DECLARE(TEST);

/// test log level
typedef enum test_log_level
{
    TEST_LOG_OFF,       ///< test logging off
    TEST_LOG_SUMMARY,   ///< print summary
    TEST_LOG_SUITE,     ///< print suite summary
    TEST_LOG_PROBLEM,   ///< print test case infos in case of problem
    TEST_LOG_ALL,       ///< print all test case infos
    TEST_LOG_INFO,      ///< print all test case infos and info messages
    TEST_LOG_LEVELS,    ///< number of test run log levels
} test_log_id;

/// test suite callback
///
/// \retval 0   success
/// \retval <0  stop test run with error
/// \retval >0  do not use, reserved for worker exit codes
typedef int (*test_suite_cb)(void);

/// test suite check callback
///
/// \retval NULL    run suite
/// \retval msg     do not run suite, msg states why
typedef const char *(*test_check_cb)(void);

/// NOP test suite which is ignored by test_run_suites().
#define TEST_SUITE_NOP ((test_suite_cb)1)

/// Unix only test suite.
#define test_suite_unix(suite) \
    UNIX_WINDOWS(suite, TEST_SUITE_NOP)

/// Windows only test suite.
#define test_suite_windows(suite) \
    UNIX_WINDOWS(TEST_SUITE_NOP, suite)


/// Initialize test run.
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int test_run_init(void);

/// Initialize test run from args.
///
/// \param argc     number of args
/// \param argv     arg list
///
/// \retval 0                   success
/// \retval -1/E_TEST_USAGE     invalid cmdline usage
/// \retval -1/E_GENERIC_OOM    out of memory
int test_run_init_from_args(int argc, char *argv[]);

/// Free test run.
///
///
void test_run_free(void);

/// Print cmdline usage.
///
/// \param name     app name (e.g. argv[0])
void test_run_print_usage(const char *name);

/// Enable or disable clean option.
///
/// \param clean    if true enable clean after test case
void test_run_enable_clean(bool clean);

/// Check whether clean option is enabled.
///
/// \retval true    clean is on
/// \retval false   clean if off
bool test_run_will_clean(void);

/// Enable or disable dump option.
///
/// \param dump    if true enable test case core dumps
void test_run_enable_dump(bool dump);

/// Check whether dump option is enabled.
///
/// \retval true    dump is on
/// \retval false   dump if off
bool test_run_will_dump(void);

/// Enable or disable fork option.
///
/// \param fork     if true enable forking for test case execution
///
/// \retval 0                   success
/// \retval -1/E_TEST_NOFORK    fork not available
int test_run_enable_fork(bool fork);

/// Check whether fork option is enabled.
///
/// \retval true    fork is on
/// \retval false   fork if off
bool test_run_will_fork(void);

/// Enable or disable skip option.
///
/// \param skip     if true enable skipping of test cases with non-normal end
void test_run_enable_skip(bool skip);

/// Check whether skip option is enabled.
///
/// \retval true    skip is on
/// \retval false   skip if off
bool test_run_will_skip(void);

/// Enable or disable stdio option.
///
/// \param stdio     if false disable stdio while exeucint test cases
void test_run_enable_stdio(bool stdio);

/// Check whether stdio option is enabled.
///
/// \retval true    stdio is on
/// \retval false   stdio if off
bool test_run_will_stdio(void);

/// Set test case timeout.
///
/// A value of 0 seconds deactivates the timeout.
///
/// \param secs     timeout in seconds
void test_run_set_timeout(size_t secs);

/// Get test case timeout.
///
/// \returns    timeout in seconds
size_t test_run_get_timeout(void);

/// Set test run log level.
///
/// \param level    log level
void test_run_set_loglevel(test_log_id level);

/// Increase test run log level by one.
///
///
void test_run_inc_loglevel(void);

/// Decrease test run log level by one.
///
///
void test_run_dec_loglevel(void);

/// Get test run log level.
///
/// \returns    log level
test_log_id test_run_get_loglevel(void);

/// Enable test run stop on first result.
///
/// \param result   min result type on which to stop
void test_run_enable_stop(test_result_id result);

/// Disable test run stop.
///
///
void test_run_disable_stop(void);

/// Get test run stop result type.
///
/// \returns                    result type
/// \retval TEST_RESULT_TYPES   stop is disabled
test_result_id test_run_get_stop(void);

/// Add test suite or test case filter.
///
/// \param filter   fnmatch pattern
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int test_run_add_filter(const char *filter);

/// Begin test suite.
///
/// \param name     test suite name
/// \param check    check callback, may be NULL
///
/// \retval 0                   success
/// \retval -1/E_TEST_DISABLED  test suite is disabled
/// \retval -1/E_TEST_MISSING   check failed, test suite is missing something to run
/// \retval -1/E_GENERIC_OOM    out of memory
int test_run_begin_suite(const char *name, test_check_cb check);

/// End test suite.
///
/// \param info     if true print info (depends also on log level)
void test_run_end_suite(bool info);

/// Begin test case.
///
/// \param name     test case name
///
/// \retval 0                   success
/// \retval -1/E_TEST_DISABLED  test case is disabled
/// \retval -1/E_GENERIC_OOM    out of memory
int test_run_begin_case(const char *name);

/// End test case.
///
/// \param info     if true print info (depends also on log level)
void test_run_end_case(bool info);

/// Run test suites.
///
/// \param name     suite name, if NULL do not create a new parent suite
/// \param ...      NULL terminated variadic list of test_suite_cb callbacks
///
/// \retval 0                   success
/// \retval >0                  worker process, cleanup and exit
/// \retval -1/E_TEST_STOP      a test case had stop result
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
int test_run_suites(const char *name, ...);

/// Run test suites.
///
/// \param name     suite name, if NULL do not create a new parent suite
/// \param check    suite check callback, may be NULL
/// \param ...      NULL terminated variadic list of test_suite_cb callbacks
///
/// \retval 0                   success
/// \retval >0                  worker process, cleanup and exit
/// \retval -1/E_TEST_STOP      a test case had stop result
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
int test_run_suites_check(const char *name, test_check_cb check, ...);

/// Run test suite of suites.
///
/// \param name     suite name, if NULL do not create a new parent suite
/// \param ap       NULL terminated variadic list of test_suite_cb callbacks
///
/// \retval 0                   success
/// \retval >0                  worker process, cleanup and exit
/// \retval -1/E_TEST_STOP      a test case had stop result
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
int test_run_suites_v(const char *name, va_list ap);

/// Run test suite of suites.
///
/// \param name     suite name, if NULL do not create a new parent suite
/// \param check    suite check callback, may be NULL
/// \param ap       NULL terminated variadic list of test_suite_cb callbacks
///
/// \retval 0                   success
/// \retval >0                  worker process, cleanup and exit
/// \retval -1/E_TEST_STOP      a test case had stop result
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
int test_run_suites_check_v(const char *name, test_check_cb check, va_list ap);

/// Run test cases.
///
/// \param name     suite name, if NULL do not create a new parent suite
/// \param ...      NULL terminated variadic list of test_case_st pointers
///
/// \retval 0                   success
/// \retval >0                  worker process, cleanup and exit
/// \retval -1/E_TEST_STOP      a test case had stop result
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
int test_run_cases(const char *name, ...);

/// Run test cases.
///
/// \param name     suite name, if NULL do not create a new parent suite
/// \param check    suite check callback, may be NULL
/// \param ...      NULL terminated variadic list of test_case_st pointers
///
/// \retval 0                   success
/// \retval >0                  worker process, cleanup and exit
/// \retval -1/E_TEST_STOP      a test case had stop result
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
int test_run_cases_check(const char *name, test_check_cb check, ...);

/// Run test cases.
///
/// \param name     suite name, if NULL do not create a new parent suite
/// \param ap       NULL terminated variadic list of test_case_st pointers
///
/// \retval 0                   success
/// \retval >0                  worker process, cleanup and exit
/// \retval -1/E_TEST_STOP      a test case had stop result
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
int test_run_cases_v(const char *name, va_list ap);

/// Run test cases.
///
/// \param name     suite name, if NULL do not create a new parent suite
/// \param check    suite check callback, may be NULL
/// \param ap       NULL terminated variadic list of test_case_st pointers
///
/// \retval 0                   success
/// \retval >0                  worker process, cleanup and exit
/// \retval -1/E_TEST_STOP      a test case had stop result
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
int test_run_cases_check_v(const char *name, test_check_cb check, va_list ap);

/// Run test case.
///
/// \param tcase    test case
///
/// \retval 0                   success
/// \retval >0                  worker process, cleanup and exit
/// \retval -1/E_TEST_STOP      test case has stop result
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
int test_run_case(const test_case_st *tcase);

/// Get number of results for a specific result type.
///
/// \param type     result type
///
/// \returns        number of results
size_t test_run_get_result(test_result_id type);

/// Get overall number of results.
///
/// \returns        number of overall results
size_t test_run_get_results(void);

/// Print test run summary.
///
///
void test_run_print_summary(void);


#endif // ifndef YTIL_TEST_RUN_H_INCLUDED
