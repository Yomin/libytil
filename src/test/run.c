/*
 * Copyright (c) 2019 Martin Rödel a.k.a. Yomin Nimoy
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

#include <ytil/test/run.h>
#include <ytil/con/vec.h>
#include <ytil/def.h>
#include <ytil/def/color.h>
#include <ytil/ext/stdlib.h>
#include <ytil/gen/path.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if OS_WINDOWS
    #include <shlwapi.h>
#else
    #include <fnmatch.h>
    #include <sys/wait.h>
    #include <sys/ptrace.h>
#endif


/// test run trace status
typedef struct test_trace
{
    bool    check;      ///< check trace status
    bool    traced;     ///< process is traced
    bool    nofork;     ///< disable fork
    bool    skip;       ///< enable skip
    bool    notimeout;  ///< disable timeout
} test_trace_st;

/// test run
typedef struct test_run
{
    path_ct         path;                       ///< suite/case path
    str_ct          name;                       ///< last suite/case name
    vec_ct          filter;                     ///< filters
    size_t          result[TEST_RESULT_TYPES];  ///< test run results
    test_config_st  config;                     ///< test run config
    test_log_id     log;                        ///< logging level
    test_result_id  stop;                       ///< stop on first result
    test_trace_st   trace;                      ///< trace status
} test_run_st;

/// test result info
typedef struct test_result_info
{
    const char  *color;     ///< result color
    const char  *caps;      ///< result name in capitals
    const char  *name;      ///< result name
} test_result_info_st;

/// test result infos
static const test_result_info_st result_info[] =
{
    [TEST_RESULT_PASS]      = { COLOR_GREEN,    "PASS", "pass"      },
    [TEST_RESULT_SKIP]      = { COLOR_CYAN,     "SKIP", "skip"      },
    [TEST_RESULT_TIMEOUT]   = { COLOR_MAGENTA,  "TIME", "timeout"   },
    [TEST_RESULT_WARN]      = { COLOR_YELLOW,   "WARN", "warn"      },
    [TEST_RESULT_MISSING]   = { COLOR_YELLOW,   "MISS", "missing"   },
    [TEST_RESULT_FAIL]      = { COLOR_RED,      "FAIL", "fail"      },
    [TEST_RESULT_ERROR]     = { COLOR_RED,      "ERRR", "error"     },
};

/// test message colors
static const char *msg_color[] =
{
    [TEST_MSG_INFO]     = "",
    [TEST_MSG_WARN]     = COLOR_YELLOW,
    [TEST_MSG_MISSING]  = COLOR_YELLOW,
    [TEST_MSG_FAIL]     = COLOR_RED,
    [TEST_MSG_ERROR]    = COLOR_RED,
};

/// test error type definition
ERROR_DEFINE_LIST(TEST,
    ERROR_INFO(E_TEST_DISABLED, "Test suite or case disabled."),
    ERROR_INFO(E_TEST_MISSING,  "Test suite is missing something to run."),
    ERROR_INFO(E_TEST_NOFORK,   "Fork not available."),
    ERROR_INFO(E_TEST_STOP,     "A test case had stop result."),
    ERROR_INFO(E_TEST_USAGE,    "Invalid cmdline usage.")
);

/// default error type for test run module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_TEST

/// global test run state
static test_run_st *run;


/// Free test run filter.
///
/// \implements vec_dtor_cb
static void test_run_vec_free_filter(vec_const_ct vec, void *elem, void *ctx)
{
    path_ct *filter = elem;

    path_free(*filter);
}

/// Clear test run.
///
///
static void test_run_clear(void)
{
    if(run->path)
        path_free(run->path);

    if(run->name)
        str_unref(run->name);

    if(run->filter)
        vec_free_f(run->filter, test_run_vec_free_filter, NULL);

    memset(run, 0, sizeof(test_run_st));
}

int test_run_init(void)
{
    if(run)
        test_run_clear();
    else if(!(run = calloc(1, sizeof(test_run_st))))
        return error_wrap_last_errno(calloc), -1;

    run->config.clean       = false;
    run->config.dump        = false;
    run->config.fork        = UNIX_WINDOWS(true, false);
    run->config.skip        = !run->config.fork;
    run->config.stdio       = false;
    run->config.timeout     = 3;
    run->log                = TEST_LOG_SUMMARY;
    run->stop               = TEST_RESULT_TYPES;
    run->trace.check        = run->config.fork;
    run->trace.nofork       = true;
    run->trace.skip         = true;
    run->trace.notimeout    = true;

    return 0;
}

/// Get result ID from name.
///
/// \param name     name to match
///
/// \returns                    result ID
/// \retval TEST_RESULT_TYPES   no result name matched
static test_result_id test_run_match_result_name(const char *name)
{
    test_result_id result;

    for(result = 0; result < TEST_RESULT_TYPES; result++)
    {
        if(!strcmp(name, result_info[result].name))
            break;
    }

    return result;
}

void test_run_print_usage(const char *name)
{
    printf("Usage: %s [path/to/suite/case [...]]"
        "\n\t[-v|--verbose]             verbose"
        "\n\t[-q|--quiet]               quiet"
        "\n\t[--[no]clean]              clean up after test cases"
        "\n\t[--[no]dump]               coredump on test case crash"
        "\n\t[--[no]fork]               fork to execute test cases"
        "\n\t[--[no]skip]               skip test cases expecting exit or signal"
        "\n\t[--[no]stdio]              do not suppress stdio"
        "\n\t[--[no]debug]              shorthand for --nofork --skip"
        "\n\t[--[no]timeout=<sec>]      test case timeout in seconds"
        "\n\t[--[no]stop[=<result>]]    stop on first result"
        "\n", name);
}

/// test run cmdline options
static const struct option test_run_options[] =
{
    { "help",       no_argument,        NULL,   'h' },
    { "verbose",    no_argument,        NULL,   'v' },
    { "quiet",      no_argument,        NULL,   'q' },
    { "clean",      no_argument,        NULL,   'c' },
    { "noclean",    no_argument,        NULL,   'C' },
    { "dump",       no_argument,        NULL,   'd' },
    { "nodump",     no_argument,        NULL,   'D' },
    { "fork",       no_argument,        NULL,   'f' },
    { "nofork",     no_argument,        NULL,   'F' },
    { "skip",       no_argument,        NULL,   's' },
    { "noskip",     no_argument,        NULL,   'S' },
    { "stdio",      no_argument,        NULL,   'i' },
    { "nostdio",    no_argument,        NULL,   'I' },
    { "debug",      no_argument,        NULL,   'b' },
    { "nodebug",    no_argument,        NULL,   'B' },
    { "timeout",    required_argument,  NULL,   't' },
    { "notimeout",  no_argument,        NULL,   'T' },
    { "stop",       optional_argument,  NULL,   'r' },
    { "nostop",     no_argument,        NULL,   'R' },
    { NULL,         0,                  NULL,    0  }
};

int test_run_init_from_args(int argc, char *argv[])
{
    test_result_id result;
    size_t timeout;
    int opt, err;

    if(test_run_init())
        return error_pass(), -1;

    err     = opterr;
    opterr  = 0;

    while((opt = getopt_long(argc, argv, "hvq", test_run_options, NULL)) != -1)
    {
        switch(opt)
        {
        case 'h':
            goto usage;

        case 'v':
            test_run_inc_loglevel();
            break;

        case 'q':
            test_run_set_loglevel(TEST_LOG_OFF);
            break;

        case 'c':
        case 'C':
            test_run_enable_clean(opt == 'c');
            break;

        case 'd':
        case 'D':
            test_run_enable_dump(opt == 'd');
            break;

        case 'f':
        case 'F':

            if(test_run_enable_fork(opt == 'f'))
                goto error;

            break;

        case 's':
        case 'S':
            test_run_enable_skip(opt == 's');
            break;

        case 'i':
        case 'I':
            test_run_enable_stdio(opt == 'i');
            break;

        case 'b':
        case 'B':

            if(test_run_enable_fork(opt != 'b'))
                goto error;

            test_run_enable_skip(opt == 'b');
            break;

        case 't':

            if(str2uz(&timeout, optarg, 10) < 0)
                goto usage;

            test_run_set_timeout(timeout);
            break;

        case 'T':
            test_run_set_timeout(0);
            break;

        case 'r':
            result = optarg ? test_run_match_result_name(optarg) : TEST_RESULT_TIMEOUT;

            if(result == TEST_RESULT_TYPES)
                goto usage;

            test_run_enable_stop(result);
            break;

        case 'R':
            test_run_disable_stop();
            break;

        default:
            goto usage;
        }
    }

    opt     = optind;
    optind  = 0;
    opterr  = err;

    for(; opt < argc; opt++)
    {
        if(test_run_add_filter(argv[opt]))
            return error_pass(), test_run_free(), -1;
    }

    return 0;

usage:
    optind  = 0;
    opterr  = err;

    test_run_free();

    return_error_if_reached(E_TEST_USAGE, -1);

error:
    optind  = 0;
    opterr  = err;

    test_run_free();

    return error_pass(), -1;
}

void test_run_free(void)
{
    if(!run)
        return;

    test_run_clear();
    test_case_free();

    free(run);
    run = NULL;
}

void test_run_enable_clean(bool clean)
{
    assert(run);

    run->config.clean = clean;
}

bool test_run_will_clean(void)
{
    assert(run);

    return run->config.clean;
}

void test_run_enable_dump(bool dump)
{
    assert(run);

    run->config.dump = dump;
}

bool test_run_will_dump(void)
{
    assert(run);

    return run->config.dump;
}

int test_run_enable_fork(bool fork)
{
    assert(run);

#if OS_WINDOWS

    return_error_if_pass(fork, E_TEST_NOFORK, -1);

#endif

    run->config.fork    = fork;
    run->trace.check    = fork;     // disable trace check on nofork
    run->trace.nofork   = false;

    return 0;
}

bool test_run_will_fork(void)
{
    assert(run);

    return run->config.fork;
}

void test_run_enable_skip(bool skip)
{
    assert(run);

    run->config.skip    = skip;
    run->trace.skip     = false;
}

bool test_run_will_skip(void)
{
    assert(run);

    return run->config.skip;
}

void test_run_enable_stdio(bool stdio)
{
    assert(run);

    run->config.stdio = stdio;
}

bool test_run_will_stdio(void)
{
    assert(run);

    return run->config.stdio;
}

void test_run_set_timeout(size_t secs)
{
    assert(run);

    run->config.timeout     = secs;
    run->trace.notimeout    = false;
}

size_t test_run_get_timeout(void)
{
    assert(run);

    return run->config.timeout;
}

void test_run_set_loglevel(test_log_id level)
{
    assert(run);
    assert(level < TEST_LOG_LEVELS);

    run->log = level;
}

void test_run_inc_loglevel(void)
{
    assert(run);

    if(run->log < TEST_LOG_LEVELS - 1)
        run->log++;
}

void test_run_dec_loglevel(void)
{
    assert(run);

    if(run->log > 0)
        run->log--;
}

test_log_id test_run_get_loglevel(void)
{
    assert(run);

    return run->log;
}

void test_run_enable_stop(test_result_id result)
{
    assert(run);
    assert(result < TEST_RESULT_TYPES);

    run->stop = result;
}

void test_run_disable_stop(void)
{
    assert(run);

    run->stop = TEST_RESULT_TYPES;
}

test_result_id test_run_get_stop(void)
{
    assert(run);

    return run->stop;
}

int test_run_add_filter(const char *filter)
{
    path_ct path;

    assert(run);
    assert(filter);

    if(!filter[0])
        return 0;

    if(!run->filter && !(run->filter = vec_new_c(2, sizeof(path_ct))))
        return error_wrap(), -1;

    if(!(path = path_new_c(filter)))
        return error_wrap(), -1;

    if(!vec_push_p(run->filter, path))
        return error_wrap(), path_free(path), -1;

    return 0;
}

/// Match filter against current path.
///
/// \implements vec_fold_cb
///
/// \retval 0                   filter does not match
/// \retval 1                   filter does match
/// \retval -1/E_GENERIC_OOM    out of memory
static int test_run_vec_match_filter(vec_const_ct vec, size_t pos, void *elem, void *ctx)
{
    path_ct *filter = elem;
    bool *complete  = ctx;
    str_ct pattern;
    int rc;

    if(*complete)
        pattern = path_get_s(*filter, PATH_STYLE_UNIX);
    else
        pattern = path_get_ns(*filter, path_depth(run->path), false, PATH_STYLE_UNIX);

    if(!pattern)
        return error_wrap(), -1;

#if OS_WINDOWS
    rc = !!PathMatchSpecA(str_c(run->name), str_c(pattern));
#else
    rc = !fnmatch(str_c(pattern), str_c(run->name), FNM_LEADING_DIR);
#endif

    str_unref(pattern);

    return rc;
}

/// Match all filters against current path.
///
/// \param complete     if true path must match whole filter, else only the prefix
///
/// \retval 0                   no filter matches
/// \retval 1                   no filter available or a filter matches
/// \retval -1/E_GENERIC_OOM    out of memory
static int test_run_match_path(bool complete)
{
    if(!run->filter)
        return 1;

    if(!(run->name = path_get_s(run->path, PATH_STYLE_UNIX)))
        return error_wrap(), -1;

    return error_pick_int(E_VEC_CALLBACK,
        vec_fold(run->filter, test_run_vec_match_filter, &complete));
}

/// End test suite or test case.
///
///
static void test_run_end(void)
{
    path_drop(run->path, 1);

    if(run->name)
    {
        str_unref(run->name);
        run->name = NULL;
    }
}

/// Begin test suite or test case.
///
/// \param name         test suite or test case name
/// \param tcase        if true begin test case else test suite
///
/// \retval 0                   success
/// \retval -1/E_TEST_DISABLED  suite or case is disabled
/// \retval -1/E_GENERIC_OOM    out of memory
static int test_run_begin(const char *name, bool tcase)
{
    path_ct path;

    assert(run);

    if(!run->path)
        path = path_new_cs(name, PATH_STYLE_UNIX);
    else
        path = path_append_cs(run->path, name, PATH_STYLE_UNIX);

    if(!path)
        return error_wrap(), -1;

    if(run->name)
        str_unref(run->name);

    run->path   = path;
    run->name   = NULL;

    switch(test_run_match_path(tcase))
    {
    case 1:
        return 0;

    case 0:
        return error_set(E_TEST_DISABLED), test_run_end(), -1;

    default:
        return error_pass(), test_run_end(), -1;
    }
}

int test_run_begin_suite(const char *name, test_check_cb check)
{
    test_result_id result = TEST_RESULT_MISSING;
    const char *msg;

    if(test_run_begin(name, false))
        return error_pass(), -1;

    if(!check || !(msg = check()))
        return 0;

    run->result[result]++;

    if(run->log >= TEST_LOG_SUITE)
    {
        if(!run->name)
            run->name = path_get_s(run->path, PATH_STYLE_UNIX);

        printf("[%s%s" COLOR_OFF "] %s: %s\n", result_info[result].color,
            result_info[result].caps, run->name ? str_c(run->name) : name, msg);
    }

    return error_set(E_TEST_MISSING), test_run_end(), -1;
}

void test_run_end_suite(bool info)
{
    assert(run);
    assert(run->path && path_depth(run->path));

    if(info && run->log >= TEST_LOG_SUITE)
    {
        if(!run->name)
            run->name = path_get_s(run->path, PATH_STYLE_UNIX);

        printf("[DONE] %s\n", run->name ? str_c(run->name) : "<suite>");
    }

    test_run_end();
}

int test_run_begin_case(const char *name)
{
    return error_pass_int(test_run_begin(name, true));
}

#define TEST_MSG_INDENT 4 ///< number of spaces to indent messages

/// Print test case message line.
///
/// The line is broken at newlines which are printed separately.
/// Text between STX and ETX is printed escaped.
///
/// \param line     line to print
/// \param level    level of line
/// \param tmp      temporary string for escaping
static void test_run_print_line(const char *line, int level, str_ct tmp)
{
    const char *ptr;

    printf("%*s", TEST_MSG_INDENT * (level + 1), "");

    for(; (ptr = strpbrk(line, "\n\x02")); line = ptr + 1)
    {
        printf("%.*s", (int)(ptr - line), line);

        if(ptr[0] == '\n')
        {
            printf("\n%*s", TEST_MSG_INDENT * (level + 1), "");
            continue;
        }

        if(!tmp)
            continue;

        line = ptr + 1;

        if((ptr = strchr(line, '\x03')))
            str_set_bs(tmp, line, ptr - line);
        else
            str_set_s(tmp, line);

        str_escape(tmp);
        printf("%s", str_c(tmp));

        if(!ptr)
        {
            printf("\n");
            return;
        }
    }

    printf("%s\n", line);
}

/// Print test case message.
///
/// \implements test_case_msg_cb
static void test_run_print_msg(const test_msg_st *msg, void *ctx)
{
    const test_call_st *call;
    const test_line_st *line;
    str_ct tmp = NULL;
    size_t i;

    return_if_fail(msg->type != TEST_MSG_INFO || run->log >= TEST_LOG_INFO);

    for(i = 0, call = msg->call; i < msg->calls; i++, call++)
    {
        printf("%*s" COLOR_MAGENTA "%s:%zu" COLOR_OFF "\n%*s%s\n",
            TEST_MSG_INDENT, "",
            call->pos.file ? call->pos.file : "<file>", call->pos.line,
            TEST_MSG_INDENT, "",
            call->call ? call->call : "<call>");
    }

    printf("%*s%s" COLOR_MAGENTA "%s:%zu" COLOR_OFF "\n",
        TEST_MSG_INDENT, "",
        msg->pos.after ? COLOR_YELLOW "after " : "",
        msg->pos.file ? msg->pos.file : "<file>", msg->pos.line);

    printf("%s", msg_color[msg->type]);

    if(msg->lines)
        tmp = str_new_l("");

    for(i = 0, line = msg->line; i < msg->lines; i++, line++)
    {
        if(line->msg)
            test_run_print_line(line->msg, line->level, tmp);
        else
            printf("\n");
    }

    if(tmp)
        str_unref(tmp);

    printf("%s\n", COLOR_OFF);
}

void test_run_end_case(bool info)
{
    test_result_id result;

    assert(run);
    assert(run->path && path_depth(run->path));

    if(!info || run->log < TEST_LOG_PROBLEM)
    {
        test_run_end();
        return;
    }

    result = test_case_result();

    if(run->log < TEST_LOG_ALL && (result == TEST_RESULT_PASS || result == TEST_RESULT_SKIP))
    {
        test_run_end();
        return;
    }

    if(!run->name)
        run->name = path_get_s(run->path, PATH_STYLE_UNIX);

    printf("[%s%s" COLOR_OFF "] %s\n", result_info[result].color,
        result_info[result].caps, run->name ? str_c(run->name) : test_case_name());

    test_case_fold_msg(test_run_print_msg, NULL);

    test_run_end();
}

int test_run_suites(const char *name, ...)
{
    va_list ap;
    int rc;

    va_start(ap, name);
    rc = test_run_suites_check_v(name, NULL, ap);
    va_end(ap);

    return error_pass_int(rc);
}

int test_run_suites_check(const char *name, test_check_cb check, ...)
{
    va_list ap;
    int rc;

    va_start(ap, check);
    rc = test_run_suites_check_v(name, check, ap);
    va_end(ap);

    return error_pass_int(rc);
}

int test_run_suites_v(const char *name, va_list ap)
{
    return error_pass_int(test_run_suites_check_v(name, NULL, ap));
}

int test_run_suites_check_v(const char *name, test_check_cb check, va_list ap)
{
    const test_suite_st *suite;
    int rc = 0;

    assert(run);

    if(name && (rc = test_run_begin_suite(name, check)))
    {
        switch(error_code(0))
        {
        case E_TEST_DISABLED:
            return 0;

        case E_TEST_MISSING:

            if(TEST_RESULT_MISSING >= run->stop)
                return error_push(E_TEST_STOP), -1;

            return 0;

        default:
            return error_pass_int(rc);
        }
    }

    while((suite = va_arg(ap, const test_suite_st *)))
    {
        if(suite == TEST_SUITE_NOP)
            continue;

        if((rc = suite->run(suite->param)))
            break;
    }

    if(name)
        test_run_end_suite(true);

    return error_pass_int(rc);
}

int test_run_cases(const char *name, ...)
{
    va_list ap;
    int rc;

    va_start(ap, name);
    rc = test_run_cases_check_v(name, NULL, ap);
    va_end(ap);

    return error_pass_int(rc);
}

int test_run_cases_check(const char *name, test_check_cb check, ...)
{
    va_list ap;
    int rc;

    va_start(ap, check);
    rc = test_run_cases_check_v(name, check, ap);
    va_end(ap);

    return error_pass_int(rc);
}

int test_run_cases_v(const char *name, va_list ap)
{
    return error_pass_int(test_run_cases_check_v(name, NULL, ap));
}

int test_run_cases_check_v(const char *name, test_check_cb check, va_list ap)
{
    const test_case_st *tcase;
    int rc = 0;

    assert(run);

    if(name && (rc = test_run_begin_suite(name, check)))
    {
        switch(error_code(0))
        {
        case E_TEST_DISABLED:
            return 0;

        case E_TEST_MISSING:

            if(TEST_RESULT_MISSING >= run->stop)
                return error_push(E_TEST_STOP), -1;

            return 0;

        default:
            return error_pass_int(rc);
        }
    }

    while((tcase = va_arg(ap, const test_case_st *)))
    {
        if(tcase == TEST_CASE_NOP)
            continue;

        if((rc = test_run_case(tcase)))
            break;
    }

    if(name)
        test_run_end_suite(true);

    return error_pass_int(rc);
}

/// Check if process is traced.
///
/// \retval 0                   success
/// \retval 1                   child, process is not traced
/// \retval 2                   child, process is traced
/// \retval -1/E_GENERIC_WRAP   wrapped errno
static int test_run_trace_check(void)
{
#if OS_WINDOWS

    return 0;

#else

    pid_t child, parent;
    int status, rc;

    if(!run->trace.check)
        return 0;

    run->trace.check = false;

    if((child = fork()) < 0)
        return error_wrap_last_errno(fork), -1;

    if(child)
    {
        if(waitpid(child, &status, 0) < 0)
            return error_wrap_last_errno(waitpid), -1;

        if(WIFEXITED(status) && WEXITSTATUS(status) == 2)
        {
            run->trace.traced   = true;
            run->config.fork    = run->trace.nofork ? false : run->config.fork;
            run->config.skip    = run->trace.skip ? true : run->config.skip;
            run->config.timeout = run->trace.notimeout ? 0 : run->config.timeout;

            if(run->log >= TEST_LOG_PROBLEM)
            {
                if(run->trace.nofork)
                    printf("[" COLOR_YELLOW "TRCE" COLOR_OFF "] --nofork\n");

                if(run->trace.skip)
                    printf("[" COLOR_YELLOW "TRCE" COLOR_OFF "] --skip\n");

                if(run->trace.notimeout)
                    printf("[" COLOR_YELLOW "TRCE" COLOR_OFF "] --notimeout\n");
            }
        }

        return 0;
    }
    else
    {
        parent = getppid();

        if(ptrace(PTRACE_ATTACH, parent, NULL, NULL) < 0)
        {
            rc = errno == EPERM ? 2 : 1;
        }
        else
        {
            waitpid(parent, NULL, 0);
            ptrace(PTRACE_DETACH, parent, NULL, NULL);
            rc = 1;
        }

        if(run->config.clean)
            return rc;

        _exit(rc);
    }

#endif // if OS_WINDOWS
}

int test_run_case(const test_case_st *tcase)
{
    test_result_id result;
    int rc;

    assert(run);
    assert(tcase);

    if((rc = test_run_begin_case(tcase->name)))
    {
        if(error_code(0) == E_TEST_DISABLED)
            return 0;

        return error_pass_int(rc);
    }

    if((rc = test_run_trace_check()) > 0) // child process
        return test_run_end_case(false), rc;

    if((rc = test_case_run(tcase, &run->config)) < 0)
        return error_pass(), test_run_end_case(false), -1;

    if(rc > 0) // worker process
        return test_run_end_case(false), rc;

    result = test_case_result();
    run->result[result]++;

    test_run_end_case(true);

    if(result >= run->stop)
        return error_set(E_TEST_STOP), -1;

    return 0;
}

size_t test_run_get_result(test_result_id type)
{
    assert(run);
    assert(type < TEST_RESULT_TYPES);

    return run->result[type];
}

size_t test_run_get_results(void)
{
    test_result_id result;
    size_t results;

    assert(run);

    for(results = 0, result = 0; result < TEST_RESULT_TYPES; result++)
        results += run->result[result];

    return results;
}

void test_run_print_summary(void)
{
    test_result_id result;
    int n;

    assert(run);

    return_if_fail(run->log >= TEST_LOG_SUMMARY);

    printf("Summary: ");

    for(n = 0, result = 0; result < TEST_RESULT_TYPES; result++)
    {
        if(run->result[result])
        {
            n += printf("%s%s%zu %s", n ? ", " : "", result_info[result].color,
                run->result[result], result_info[result].name);
        }
    }

    if(!n)
        printf(COLOR_YELLOW "no test cases executed");

    printf(COLOR_OFF "\n");
}
