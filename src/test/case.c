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

#include <ytil/test/case.h>
#include <ytil/con/vec.h>
#include <ytil/def.h>
#include <ytil/ext/stdio.h>
#include <ytil/ext/time.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "com.h"

#if OS_WINDOWS
    #include <windows.h>
#else
    #include <poll.h>
    #include <sys/resource.h>
    #include <sys/socket.h>
    #include <sys/wait.h>
#endif


/// fold state
typedef struct test_fold
{
    test_case_msg_cb    msg;    ///< msg fold callback
    void                *ctx;   ///< context
} test_fold_st;

/// test case message
typedef struct test_case_msg
{
    test_pos_st pos;    ///< message origin
    test_msg_id type;   ///< message type
    vec_ct      call;   ///< call stack
    vec_ct      line;   ///< message lines
} test_case_msg_st;

#if OS_UNIX

/// test system infos
typedef struct test_sys
{
    struct rlimit   core;       ///< coredump limits
    int             stdin;      ///< original stdin
    int             stdout;     ///< original stdout
    int             stderr;     ///< original stderr
    int             null;       ///< /dev/null
} test_sys_st;

#endif

/// test state
typedef struct test_state
{
    const test_case_st      *tcase;     ///< test case
    const test_config_st    *config;    ///< test config

    int     com;                        ///< com socket for fork mode
    jmp_buf jmp;                        ///< jump buffer for non-fork mode

    UNIX_MEMBER(test_sys_st sys)        ///< system infos

    test_pos_st     pos;                ///< current position
    vec_ct          msg;                ///< messages
    vec_ct          call;               ///< call stack
    test_result_id  result;             ///< result
} test_state_st;

/// global test state
static test_state_st *state;


/// Set test position.
///
/// \param file     file
/// \param line     file line
/// \param after    position is after line
static void test_state_set_pos(const char *file, size_t line, bool after)
{
    free(state->pos.file);

    state->pos.file     = strdup(file); // ignore error
    state->pos.line     = line;
    state->pos.after    = after;
}

void test_case_begin_test(const char *file, size_t line)
{
    assert(state);
    assert(file);

    if(state->com >= 0)
        test_com_set_pos(state->com, file, line, false);
    else
        test_state_set_pos(file, line, false);
}

void test_case_end_test(const char *file, size_t line)
{
    assert(state);
    assert(file);

    if(state->com >= 0)
        test_com_set_pos(state->com, file, line, true);
    else
        test_state_set_pos(file, line, true);
}

void test_case_abort(void)
{
    assert(state);

    longjmp(state->jmp, 1);
}

void test_case_abort_missing(const char *file, size_t line, const char *msg, ...)
{
    va_list ap;

    assert(state);

    va_start(ap, msg);
    test_case_add_msg_v(file, line, TEST_MSG_MISSING, msg, ap);
    va_end(ap);

    longjmp(state->jmp, 1);
}

void test_case_abort_fail(const char *file, size_t line, const char *msg, ...)
{
    va_list ap;

    assert(state);

    va_start(ap, msg);
    test_case_add_msg_v(file, line, TEST_MSG_FAIL, msg, ap);
    va_end(ap);

    longjmp(state->jmp, 1);
}

void test_case_abort_fail_b(const char *file, size_t line, const char *msg, ...)
{
    va_list ap;

    assert(state);

    va_start(ap, msg);
    test_case_add_msg_v(file, line, TEST_MSG_FAIL, msg, ap);
    va_end(ap);

    test_case_append_backtrace(0);

    longjmp(state->jmp, 1);
}

/// Push call on call stack.
///
/// \param exact    assume state position is exact
/// \param text     call text
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int test_state_push_call(bool exact, const char *text)
{
    test_call_st *call;

    if(!state->call && !(state->call = vec_new(2, sizeof(test_call_st))))
        return error_wrap(), -1;

    if(!(call = vec_push(state->call)))
        return error_wrap(), -1;

    call->pos.file  = state->pos.file ? strdup(state->pos.file) : NULL; // ignore error
    call->pos.line  = state->pos.line;
    call->pos.after = exact ? false : state->pos.after;
    call->call      = strdup(text); // ignore error

    return 0;
}

void test_case_push_call(const char *file, size_t line, const char *call)
{
    assert(state);
    assert(file);
    assert(call);

    if(state->com >= 0)
    {
        test_com_set_pos(state->com, file, line, true);
        test_com_push_call(state->com, call);
    }
    else
    {
        test_state_set_pos(file, line, true);
        test_state_push_call(true, call);
    }
}

/// Clone call.
///
/// \implements vec_clone_cb
static int test_state_vec_clone_call(vec_const_ct vec, void *dst, const void *src, void *ctx)
{
    const test_call_st *csrc    = src;
    test_call_st *cdst          = dst;

    cdst->pos.file  = csrc->pos.file ? strdup(csrc->pos.file) : NULL; // ignore error
    cdst->pos.line  = csrc->pos.line;
    cdst->pos.after = csrc->pos.after;
    cdst->call      = csrc->call ? strdup(csrc->call) : NULL; // ignore error

    return 0;
}

/// Free call.
///
/// \implements vec_dtor_cb
static void test_state_vec_free_call(vec_const_ct vec, void *elem, void *ctx)
{
    test_call_st *call = elem;

    free(call->pos.file);
    free(call->call);
}

/// Pop call from call stack.
///
///
static void test_state_pop_call(void)
{
    if(state->call)
        vec_pop_f(state->call, test_state_vec_free_call, NULL);
}

void test_case_pop_call(void)
{
    assert(state);

    if(state->com >= 0)
        test_com_pop_call(state->com);
    else
        test_state_pop_call();
}

/// Set test result.
///
/// \param result   result
static void test_state_set_result(test_result_id result)
{
    state->result = MAX(state->result, result);
}

/// Append test message.
///
/// \param level    message level
/// \param msg      message text, may be NULL
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int test_state_append_msg(int level, const char *msg)
{
    test_case_msg_st *cmsg;
    test_line_st *line;

    if(!state->msg || !(cmsg = vec_last(state->msg)))
        return 0; // ignore

    if(!cmsg->line && !(cmsg->line = vec_new(1, sizeof(test_line_st))))
        return error_wrap(), -1;

    if(!(line = vec_push(cmsg->line)))
        return error_wrap(), -1;

    line->msg   = msg && msg[0] ? strdup(msg) : NULL; // ignore error
    line->level = level;

    return 0;
}

/// Add test message.
///
/// \param type     message type
/// \param exact    assume state position is exact
/// \param msg      message text
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int test_state_add_msg(test_msg_id type, bool exact, const char *msg)
{
    test_case_msg_st *cmsg;

    switch(type)
    {
    case TEST_MSG_INFO:
        break;

    case TEST_MSG_WARN:
        test_state_set_result(TEST_RESULT_WARN);
        break;

    case TEST_MSG_MISSING:
        test_state_set_result(TEST_RESULT_MISSING);
        break;

    case TEST_MSG_FAIL:
        test_state_set_result(TEST_RESULT_FAIL);
        break;

    case TEST_MSG_ERROR:
        test_state_set_result(TEST_RESULT_ERROR);
        break;

    default:
        abort();
    }

    if(!state->msg && !(state->msg = vec_new(2, sizeof(test_case_msg_st))))
        return error_wrap(), -1;

    if(!(cmsg = vec_push(state->msg)))
        return error_wrap(), -1;

    if(test_state_append_msg(0, msg))
        return error_pass(), vec_pop(state->msg), -1;

    cmsg->type      = type;
    cmsg->pos.file  = state->pos.file ? strdup(state->pos.file) : NULL; // ignore error
    cmsg->pos.line  = state->pos.line;
    cmsg->pos.after = exact ? false : state->pos.after;

    if(state->call && !vec_is_empty(state->call))
        cmsg->call = vec_clone_f(state->call, test_state_vec_clone_call, test_state_vec_free_call, NULL); // ignore error

    return 0;
}

/// Free msg line.
///
/// \implements vec_dtor_cb
static void test_state_vec_free_line(vec_const_ct vec, void *elem, void *ctx)
{
    test_line_st *line = elem;

    free(line->msg);
}

/// Free msg.
///
/// \implements vec_dtor_cb
static void test_state_vec_free_msg(vec_const_ct vec, void *elem, void *ctx)
{
    test_case_msg_st *msg = elem;

    free(msg->pos.file);

    if(msg->call)
        vec_free_f(msg->call, test_state_vec_free_call, NULL);

    vec_free_f(msg->line, test_state_vec_free_line, NULL);
}

void test_case_add_msg(const char *file, size_t line, test_msg_id type, const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    test_case_add_msg_v(file, line, type, msg, ap);
    va_end(ap);
}

void test_case_add_msg_v(const char *file, size_t line, test_msg_id type, const char *msg, va_list ap)
{
    assert(state);
    assert(file);
    assert(type < TEST_MSG_TYPES);
    assert(msg);

    if(state->com >= 0)
    {
        test_com_set_pos(state->com, file, line, true);
        test_com_add_msg(state->com, type, VFMT(msg, ap));
    }
    else
    {
        test_state_set_pos(file, line, true);
        test_state_add_msg(type, true, VFMT(msg, ap));
    }
}

void test_case_append_msg(int level, const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    test_case_append_msg_v(level, msg, ap);
    va_end(ap);
}

void test_case_append_msg_v(int level, const char *msg, va_list ap)
{
    assert(state);
    assert(level >= 0);

    if(state->com >= 0)
        test_com_append_msg(state->com, level, msg ? VFMT(msg, ap) : NULL);
    else
        test_state_append_msg(level, msg ? VFMT(msg, ap) : NULL);
}

void test_case_append_backtrace(int level)
{
    const char *func;
    size_t e;

    assert(state);
    assert(level >= 0);

    test_case_append_msg(level, NULL);

    for(e = 0; e < error_depth(); e++)
    {
        func = error_stack_get_func(e);

        if(error_stack_get_type(e) == ERROR_TYPE(GENERIC))
        {
            switch(error_stack_get_code(e))
            {
            case E_GENERIC_WRAP:
                test_case_append_msg(level, "%02zu %s: <wrap>", e, func);
                continue;

            case E_GENERIC_PASS:
                test_case_append_msg(level, "%02zu %s: <pass>", e, func);
                continue;

            case E_GENERIC_SKIP:
                test_case_append_msg(level, "%02zu %s: <skip>", e, func);
                continue;

            default:
                break;
            }
        }

        test_case_append_msg(level, "%02zu %s: %s", e, func, error_stack_get_name(e));
        test_case_append_msg(level + 1, "%s", error_stack_get_desc(e));
    }

    if(!e)
        test_case_append_msg(level, "<empty_backtrace>");
}

/// Fold test case message.
///
/// \implements vec_fold_cb
static int test_case_vec_fold_msg(vec_const_ct vec, size_t pos, void *elem, void *ctx)
{
    test_case_msg_st *cmsg  = elem;
    test_fold_st *fold      = ctx;
    test_msg_st msg         = { 0 };

    msg.pos     = cmsg->pos;
    msg.type    = cmsg->type;
    msg.call    = cmsg->call ? vec_at(cmsg->call, 0) : NULL;
    msg.calls   = cmsg->call ? vec_size(cmsg->call) : 0;
    msg.line    = vec_at(cmsg->line, 0);
    msg.lines   = vec_size(cmsg->line);

    fold->msg(&msg, fold->ctx);

    return 0;
}

void test_case_fold_msg(test_case_msg_cb fold, const void *ctx)
{
    test_fold_st fold_state = { .msg = fold, .ctx = (void *)ctx };

    assert(state);
    assert(fold);

    if(state->msg)
        vec_fold(state->msg, test_case_vec_fold_msg, &fold_state);
}

/// Execute test case.
///
///
static void test_case_exec(void)
{
    if(state->tcase->setup)
    {
        if(setjmp(state->jmp)) // returns 1 on abort
            return;

        state->tcase->setup();
    }

    if(!setjmp(state->jmp)) // returns 1 on abort
        state->tcase->cb();

    if(state->tcase->teardown)
    {
        if(!setjmp(state->jmp)) // returns 1 on abort
            state->tcase->teardown();
    }
}

/// Disable core dumps.
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_WRAP   wrapped errno
static int test_case_disable_coredump(void)
{
#if OS_UNIX

    struct rlimit limit;

    if(getrlimit(RLIMIT_CORE, &state->sys.core))
        return error_wrap_last_errno(getrlimit), -1;

    if(state->sys.core.rlim_cur)
    {
        limit           = state->sys.core;
        limit.rlim_cur  = 0;

        if(setrlimit(RLIMIT_CORE, &limit))
            return error_wrap_last_errno(setrlimit), -1;
    }

#endif

    return 0;
}

/// Enable core dumps.
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_WRAP   wrapped errno
static int test_case_enable_coredump(void)
{
#if OS_UNIX

    if(setrlimit(RLIMIT_CORE, &state->sys.core))
        return error_wrap_last_errno(setrlimit), -1;

#endif

    return 0;
}

/// Disable stdio.
///
/// \param backup   if true dup stdio to backup
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_WRAP   wrapped errno
static int test_case_disable_stdio(bool backup)
{
    fflush(stdin);
    fflush(stdout);
    fflush(stderr);

#if OS_WINDOWS

    FreeConsole();

#else

    if(backup)
    {
        if(state->sys.stdin < 0 && (state->sys.stdin = dup(STDIN_FILENO)) < 0)
            return error_wrap_last_errno(dup), -1;

        if(state->sys.stdout < 0 && (state->sys.stdout = dup(STDOUT_FILENO)) < 0)
            return error_wrap_last_errno(dup), -1;

        if(state->sys.stderr < 0 && (state->sys.stderr = dup(STDERR_FILENO)) < 0)
            return error_wrap_last_errno(dup), -1;
    }

    if(state->sys.null < 0 && (state->sys.null = open("/dev/null", O_RDWR)) < 0)
        return error_wrap_last_errno(open), -1;

    if(dup2(state->sys.null, STDIN_FILENO) < 0)
        return error_wrap_last_errno(dup2), -1;

    if(dup2(state->sys.null, STDOUT_FILENO) < 0)
        return error_wrap_last_errno(dup2), -1;

    if(dup2(state->sys.null, STDERR_FILENO) < 0)
        return error_wrap_last_errno(dup2), -1;

#endif // OS_WINDOWS

    return 0;
}

/// Enable stdio.
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_WRAP   wrapped errno
static int test_case_enable_stdio(void)
{
    fflush(stdin);
    fflush(stdout);
    fflush(stderr);

#if OS_WINDOWS

    AttachConsole(ATTACH_PARENT_PROCESS);

#else

    if(dup2(state->sys.stdin, STDIN_FILENO) < 0)
        return error_wrap_last_errno(dup2), -1;

    if(dup2(state->sys.stdout, STDOUT_FILENO) < 0)
        return error_wrap_last_errno(dup2), -1;

    if(dup2(state->sys.stderr, STDERR_FILENO) < 0)
        return error_wrap_last_errno(dup2), -1;

#endif

    return 0;
}

#if OS_UNIX

/// Test case worker.
///
/// \param com      com socket
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_WRAP   wrapped errno
static int test_case_worker(int com)
{
    // put process in its own process group
    if(setpgid(0, 0))
        return error_wrap_last_errno(setpgid), -1;

    if(!state->config->dump && test_case_disable_coredump())
        return error_pass(), -1;

    if(!state->config->stdio && test_case_disable_stdio(false))
        return error_pass(), -1;

    state->com = com;

    test_case_exec();

    return 0;
}

/// Handle worker message.
///
/// \implements test_com_data_cb
static void test_case_handle_msg(test_com_data_id type, const test_com_data_un *data, void *ctx)
{
    switch(type)
    {
    case TEST_COM_POS:
        test_state_set_pos(data->pos.file, data->pos.line, data->pos.after);
        break;

    case TEST_COM_CALL_PUSH:
        test_state_push_call(true, data->call_push.call);
        break;

    case TEST_COM_CALL_POP:
        test_state_pop_call();
        break;

    case TEST_COM_MSG_ADD:
        test_state_add_msg(data->msg_add.type, true, data->msg_add.msg);
        break;

    case TEST_COM_MSG_APPEND:
        test_state_append_msg(data->msg_append.level, data->msg_append.msg);
        break;

    default:
        abort();
    }
}

/// Evaluate worker exit.
///
/// \param status   worker process exit status
static void test_case_eval_status(int status)
{
    if(WIFSIGNALED(status))
    {
        if(state->tcase->end_type != TEST_END_SIGNAL)
        {
            test_state_add_msg(TEST_MSG_ERROR, false,
                FMT("test aborted with signal %d (%s)",
                    WTERMSIG(status), strsignal(WTERMSIG(status))));
        }
        else if(WTERMSIG(status) != state->tcase->end_value)
        {
            test_state_add_msg(TEST_MSG_FAIL, false,
                FMT("got signal %d (%s), expected %d (%s)",
                    WTERMSIG(status), strsignal(WTERMSIG(status)),
                    state->tcase->end_value, strsignal(state->tcase->end_value)));
        }
        else
        {
            // pass
        }
    }
    else if(WIFEXITED(status))
    {
        if(state->tcase->end_type == TEST_END_NORMAL)
        {
            // pass
        }
        else if(state->tcase->end_type == TEST_END_SIGNAL)
        {
            test_state_add_msg(TEST_MSG_FAIL, false,
                FMT("got exit rc %d, expected signal %d (%s)",
                    WEXITSTATUS(status),
                    state->tcase->end_value, strsignal(state->tcase->end_value)));
        }
        else if(WEXITSTATUS(status) != state->tcase->end_value)
        {
            test_state_add_msg(TEST_MSG_FAIL, false,
                FMT("got exit rc %d, expected %d",
                    WEXITSTATUS(status), state->tcase->end_value));
        }
        else
        {
            // pass
        }
    }
    else
    {
        abort();
    }
}

/// Kill worker process
///
/// \param worker   worker PID
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_WRAP   wrapped errno
static int test_case_kill(pid_t worker)
{
    if(killpg(worker, SIGKILL))
        return error_wrap_last_errno(killpg), -1;

    if(waitpid(worker, NULL, 0) < 0)
        return error_wrap_last_errno(waitpid), -1;

    return 0;
}

/// Test case control.
///
/// Wait for worker to send messages and exit.
/// In case of timeout, kill worker process.
/// in case of success, evaluate process exit status.
///
/// \param com      com socket
/// \param worker   worker PID
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
static int test_case_control(int com, pid_t worker)
{
    struct pollfd pfds[1];
    timespec_st start, end, timeout, diff;
    int rc, status, poll_timeout = -1;

    pfds[0].fd      = com;
    pfds[0].events  = POLLIN;

    test_com_clear();

    if(state->config->timeout) // a timeout of 0 seconds disables it
    {
        time_ts_set_sec(&timeout, MIN((size_t)INT_MAX, state->config->timeout));
        poll_timeout = time_ts_get_milli(&timeout);

        if(clock_gettime(CLOCK_MONOTONIC, &start))
            return error_wrap_last_errno(clock_gettime), test_case_kill(worker), -1;
    }

    while(1)
    {
        if((rc = poll(pfds, 1, poll_timeout)) < 0)
            return error_wrap_last_errno(poll), test_case_kill(worker), -1;

        if(!rc) // timeout
            break;

        if(pfds[0].revents & POLLIN)
        {
            if(test_com_recv(com, test_case_handle_msg, NULL) && error_code(0) != ENODATA)
                return error_wrap(), test_case_kill(worker), -1;
        }

        if(pfds[0].revents & POLLHUP)
        {
            if(waitpid(worker, &status, 0) < 0)
                return error_wrap_last_errno(waitpid), test_case_kill(worker), -1;

            test_case_eval_status(status);

            return 0;
        }

        if(pfds[0].revents & POLLERR)
            abort();

        if(!state->config->timeout) // no timeout
            continue;

        if(clock_gettime(CLOCK_MONOTONIC, &end))
            return error_wrap_last_errno(clock_gettime), test_case_kill(worker), -1;

        time_ts_set_diff(&diff, &start, &end);

        if(time_ts_cmp(&diff, &timeout) >= 0) // timeout
            break;

        time_ts_set_diff(&timeout, &diff, &timeout);
        poll_timeout    = time_ts_get_milli(&timeout);
        start           = end;
    }

    test_state_set_result(TEST_RESULT_TIMEOUT);

    test_case_kill(worker);

    return 0;
}

/// Execute test case by forking.
///
/// Create pair of unix sockets to communicate between control and worker process.
/// Fork to create worker process which runs test_case_worker.
/// Parent process runs test_case_control.
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
/// \retval -1/E_GENERIC_WRAP   wrapped errno
static int test_case_fork(void)
{
    int sv[2], rc;
    pid_t pid;

    if(socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sv) == -1)
        return error_wrap_last_errno(socketpair), -1;

    fflush(stdin);
    fflush(stdout);
    fflush(stderr);

    if((pid = fork()) == -1)
    {
        error_wrap_last_errno(fork);

        close(sv[0]);
        close(sv[1]);

        return -1;
    }

    if(!pid)
    {
        close(sv[0]);

        test_case_worker(sv[1]);

        close(sv[1]);

        if(!state->config->clean)
            _exit(0);

        return 1;
    }
    else
    {
        close(sv[1]);

        rc = error_pass_int(test_case_control(sv[0], pid));

        close(sv[0]);

        if(!state->tcase->cleanup)
            return rc;

        if(!state->config->stdio)
            test_case_disable_stdio(true);

        if(!setjmp(state->jmp)) // returns on abort with 1
            state->tcase->cleanup();

        if(!state->config->stdio)
            test_case_enable_stdio();

        return rc;
    }
}

#endif // if OS_UNIX

/// Execute test case in runner process.
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_WRAP   wrapped errno
static int test_case_nofork(void)
{
    if(!state->config->dump && test_case_disable_coredump())
        return error_pass(), -1;

    if(!state->config->stdio && test_case_disable_stdio(true))
        return error_pass(), -1;

    test_case_exec();

    if(state->tcase->cleanup)
    {
        if(!setjmp(state->jmp)) // returns on abort with 1
            state->tcase->cleanup();
    }

    if(!state->config->stdio && test_case_enable_stdio())
        return error_pass(), -1;

    if(!state->config->dump && test_case_enable_coredump())
        return error_pass(), -1;

    return 0;
}

/// Initialize test state.
///
/// \param tcase    test case
/// \param config   test config
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int test_state_init(const test_case_st *tcase, const test_config_st *config)
{
    test_state_st init = { 0 };

    init.tcase  = tcase;
    init.config = config;
    init.com    = -1;

#if OS_UNIX
    init.sys.stdin  = -1;
    init.sys.stdout = -1;
    init.sys.stderr = -1;
    init.sys.null   = -1;
#endif

    if(state)
    {
        UNIX(init.sys = state->sys);

        free(state->pos.file);

        if(state->msg)
        {
            vec_clear_f(state->msg, test_state_vec_free_msg, NULL);
            init.msg = state->msg;
        }

        if(state->call)
        {
            vec_clear_f(state->call, test_state_vec_free_call, NULL);
            init.call = state->call;
        }
    }
    else if(!(state = calloc(1, sizeof(test_state_st))))
    {
        return error_wrap_last_errno(calloc), -1;
    }

    *state = init;

    return 0;
}

int test_case_run(const test_case_st *tcase, const test_config_st *config)
{
    assert(tcase);
    assert(tcase->end_type < TEST_END_TYPES);
    assert(config);

    if(test_state_init(tcase, config))
        return error_pass(), -1;

    if(config->skip && tcase->end_type != TEST_END_NORMAL)
        return test_state_set_result(TEST_RESULT_SKIP), 0;

#if OS_UNIX

    if(config->fork)
        return error_pass_int(test_case_fork());

#endif

    return error_pass_int(test_case_nofork());
}

const char *test_case_name(void)
{
    assert(state);

    return state->tcase->name;
}

test_result_id test_case_result(void)
{
    assert(state);

    return state->result;
}

void test_case_free(void)
{
    if(!state)
        return;

    free(state->pos.file);

    if(state->msg)
        vec_free_f(state->msg, test_state_vec_free_msg, NULL);

    if(state->call)
        vec_free_f(state->call, test_state_vec_free_call, NULL);

#if OS_UNIX

    if(state->sys.stdin >= 0)
        close(state->sys.stdin);

    if(state->sys.stdout >= 0)
        close(state->sys.stdout);

    if(state->sys.stderr >= 0)
        close(state->sys.stderr);

    if(state->sys.null >= 0)
        close(state->sys.null);

#endif

    test_com_free();

    free(state);
    state = NULL;
}
