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

#include <ytil/test/run.h>
#include <ytil/test/com.h>
#include <ytil/test/state.h>
#include <ytil/test/ctx.h>
#include <ytil/con/vec.h>
#include <ytil/sys/proc.h>
#include <ytil/def.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ptrace.h>
#include <sys/resource.h>

#define COLOR_DEFAULT   "\033[0m"
#define COLOR_RED       "\033[1;31;40m"
#define COLOR_GREEN     "\033[1;32;40m"
#define COLOR_YELLOW    "\033[1;33;40m"


enum test_run_trace_status
{
      TEST_PARENT_NOT_TRACED
    , TEST_PARENT_TRACED
    , TEST_CHILD_NOT_TRACED
    , TEST_CHILD_TRACED
};

typedef enum test_run_loglvl
{
      TEST_LOGLVL_SUMMARY
    , TEST_LOGLVL_SUITE
    , TEST_LOGLVL_CASE
} test_run_loglvl_id;

typedef struct test_count
{
    size_t results[TEST_RESULTS], asserts;
} test_count_st;

typedef struct test_run
{
    clockid_t control_clock, worker_clock;
    test_state_ct state;
    test_com_ct com;
    test_count_st count;
    vec_ct path, filters;
    FILE *fp;
    bool fork, clean_exit, core_dump, skip, check_traced, traced;
    test_run_loglvl_id loglvl;
} test_run_st;

typedef struct test_filter
{
    vec_ct path;
    bool matched;
    size_t level;
} test_filter_st;

typedef struct test_filter_node
{
    vec_ct units;
} test_filter_node_st;

typedef struct test_filter_unit
{
    char *name;
    size_t len;
    bool prefix;
} test_filter_unit_st;

typedef struct test_match_state
{
    size_t level, matched;
    const char *name;
    test_entry_id type;
} test_match_st;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_TEST_RUN_INVALID_OBJECT, "Invalid test run object.")
    , ERROR_INFO(E_TEST_RUN_INVALID_SUITE, "Invalid suite.")
    , ERROR_INFO(E_TEST_RUN_INVALID_FILTER, "Invalid filter.")
    , ERROR_INFO(E_TEST_RUN_INVALID_OPTION, "Invalid option.")
    , ERROR_INFO(E_TEST_RUN_MISSING_ARG, "Missing argument.")
    , ERROR_INFO(E_TEST_RUN_MALFORMED_ARG, "Malformed argument.")
    , ERROR_INFO(E_TEST_RUN_TRACE_CHECK, "Trace check failed.")
};


static void fperror(FILE *fp, const char *msg)
{
    fprintf(fp, "%s: %s\n", msg, error_origin_get_desc());
}

static int test_run_get_clock(clockid_t *clock, ...)
{
    timespec_st ts;
    va_list ap;
    
    va_start(ap, clock);
    
    while(1)
    {
        *clock = va_arg(ap, clockid_t);
        
        if(!clock_gettime(*clock, &ts))
            return va_end(ap), 0;
        
        if(*clock == CLOCK_REALTIME)
            return va_end(ap), error_set_errno(clock_gettime), -1;
    }
}

static const struct option test_run_options[] =
{
      { "help",    no_argument,       NULL, 'h' }
    , { "fork",    no_argument,       NULL, 'f' }
    , { "nofork",  no_argument,       NULL, 'F' }
    , { "clean",   no_argument,       NULL, 'c' }
    , { "noclean", no_argument,       NULL, 'C' }
    , { "dump",    no_argument,       NULL, 'd' }
    , { "nodump",  no_argument,       NULL, 'D' }
    , { "skip",    no_argument,       NULL, 's' }
    , { "noskip",  no_argument,       NULL, 'S' }
    , { "loglvl",  required_argument, NULL, 'l' }
    , { NULL,      0,                 NULL,  0  }
};

static int test_run_config(test_run_ct run, int argc, char *argv[])
{
    int opt;
    
    if(proc_init_title(argc, argv))
        fperror(run->fp, "failed to init proc title");
    
    if(test_run_get_clock(&run->control_clock,
        CLOCK_MONOTONIC_RAW, CLOCK_MONOTONIC, CLOCK_REALTIME)
    || test_run_get_clock(&run->worker_clock,
        CLOCK_PROCESS_CPUTIME_ID, CLOCK_MONOTONIC_RAW, CLOCK_MONOTONIC, CLOCK_REALTIME))
            return error_propagate(), fperror(run->fp, "failed to get clock"), -1;
    
    if(argc <= 1)
        return 0;
    
    while((opt = getopt_long(argc, argv, ":hv", test_run_options, NULL)) != -1)
        switch(opt)
        {
        case 'h':
            fprintf(run->fp, "Usage: %s [-v] [--loglvl=<lvl>] [--[no]fork] [--[no]clean] [--[no]dump] [--[no]skip] [path/to1,to2/suite*/case [...]]\n", argv[0]);
            return 1;
        case 'f':
        case 'F':
            test_run_enable_fork(run, opt == 'f');
            break;
        case 'c':
        case 'C':
            run->clean_exit = opt == 'c';
            break;
        case 'd':
        case 'D':
            run->core_dump = opt == 'd';
            break;
        case 's':
        case 'S':
            run->skip = opt == 's';
            break;
        case 'l':
            if(strspn(optarg, "1234567890") != strlen(optarg))
            {
                fprintf(run->fp, "loglvl argument not numeric\n");
                return error_set(E_TEST_RUN_MALFORMED_ARG), -1;
            }
            run->loglvl = atoi(optarg);
            break;
        case 'v':
            run->loglvl++;
            break;
        case '?':
            fprintf(run->fp, "invalid option '%s'\n", argv[optind-1]);
            return error_set(E_TEST_RUN_INVALID_OPTION), -1;
        case ':':
            fprintf(run->fp, "missing argument for '%s'\n", argv[optind-1]);
            return error_set(E_TEST_RUN_MISSING_ARG), -1;
        default:
            abort();
        }
    
    for(; optind < argc; optind++)
        if(test_run_add_filter(run, argv[optind]))
        {
            if(error_check(E_TEST_RUN_INVALID_FILTER))
                fprintf(run->fp, "invalid filter '%s'\n", argv[optind]);
            else
                fperror(run->fp, "failed to add filter");
            
            return error_propagate(), -1;
        }
    
    return 0;
}

static int test_run_msg(test_com_msg_id type, test_com_msg_un *msg, void *ctx);

test_run_ct test_run_new(void)
{
    test_run_ct run;
    
    if(!(run = test_run_new_with_args(0, NULL)))
        return error_propagate(), NULL;
    
    return run;
}

test_run_ct test_run_new_with_args(int argc, char *argv[])
{
    test_run_ct run;
    
    if(!(run = calloc(1, sizeof(test_run_st))))
        return error_set_errno(calloc), perror("failed to init test run"), NULL;
    
    run->fp = stdout;
    run->fork = true;
    run->skip = true;
    run->check_traced = true;
    
    if(test_run_config(run, argc, argv))
        return error_propagate(), test_run_free(run), NULL;
    
    if(!(run->state = test_state_new()))
        return error_push(), fperror(run->fp, "failed to init test state"), test_run_free(run), NULL;
    
    if(!(run->com = test_com_new(test_run_msg, run)))
        return error_push(), fperror(run->fp, "failed to init com"), test_run_free(run), NULL;
    
    return run;
}

static void test_run_free_filter_unit(vec_const_ct vec, void *elem, void *ctx)
{
    test_filter_unit_st *unit = elem;
    
    if(unit->name)
        free(unit->name);
}

static void test_run_free_filter_node(vec_const_ct vec, void *elem, void *ctx)
{
    test_filter_node_st *node = elem;
    
    if(node->units)
        vec_free_f(node->units, test_run_free_filter_unit, NULL);
}

static void test_run_free_filter(vec_const_ct vec, void *elem, void *ctx)
{
    test_filter_st *filter = elem;
    
    if(filter->path)
        vec_free_f(filter->path, test_run_free_filter_node, NULL);
}

void test_run_free(test_run_ct run)
{
    assert(run);
    
    if(run->filters)
        vec_free_f(run->filters, test_run_free_filter, NULL);
    
    if(run->state)
        test_state_free(run->state);
    
    if(run->com)
        test_com_free(run->com);
    
    proc_free_title();
    
    free(run);
}

void test_run_enable_fork(test_run_ct run, bool fork)
{
    return_if_fail(run);
    
    run->fork = fork;
    run->check_traced = false;
}

int test_run_add_filter(test_run_ct run, const char *text)
{
    test_filter_st *filter;
    test_filter_node_st *node;
    test_filter_unit_st *unit;
    const char *start_node, *start_unit, *end_node, *end_unit;
    size_t len_node, len_unit;
    
    return_error_if_fail(run, E_TEST_RUN_INVALID_OBJECT, -1);
    return_error_if_fail(text, E_TEST_RUN_INVALID_FILTER, -1);
    
    if(text[0] == '/')
        text++;
    
    return_error_if_fail(text[0], E_TEST_RUN_INVALID_FILTER, -1);
    
    if(!run->filters && !(run->filters = vec_new(2, sizeof(test_filter_st))))
        return error_push(), -1;
    
    if(!(filter = vec_push(run->filters))
    || !(filter->path = vec_new(3, sizeof(test_filter_node_st))))
        return error_push(), vec_pop_f(run->filters, test_run_free_filter, NULL), -1;
    
    for(start_node = text, end_node = strchr(start_node, '/');
        start_node;
        start_node = end_node ? end_node+1 : NULL,
        end_node = end_node ? strchr(start_node, '/') : NULL)
    {
        len_node = end_node ? (size_t)(end_node - start_node) : strlen(start_node);
        
        if(!(node = vec_push(filter->path))
        || !(node->units = vec_new(2, sizeof(test_filter_unit_st))))
            return error_push(), vec_pop_f(run->filters, test_run_free_filter, NULL), -1;
        
        for(start_unit = start_node, end_unit = memchr(start_unit, ',', len_node);
            start_unit;
            len_node -= end_unit ? len_unit+1 : 0,
            start_unit = end_unit ? end_unit+1 : NULL,
            end_unit = end_unit ? memchr(start_unit, ',', len_node) : NULL)
        {
            len_unit = end_unit ? (size_t)(end_unit - start_unit) : len_node;
            
            if(!len_unit)
                return error_set(E_TEST_RUN_INVALID_FILTER), vec_pop_f(run->filters, test_run_free_filter, NULL), -1;
            
            if(!(unit = vec_push(node->units)))
                return error_push(), vec_pop_f(run->filters, test_run_free_filter, NULL), -1;
            
            unit->len = len_unit;
            
            if(start_unit[len_unit-1] == '*')
            {
                unit->prefix = true;
                unit->len--;
            }
            
            if(memchr(start_unit, '*', unit->len))
                return error_set(E_TEST_RUN_INVALID_FILTER), vec_pop_f(run->filters, test_run_free_filter, NULL), -1;
            
            if(unit->len && !(unit->name = strndup(start_unit, unit->len)))
                return error_set_errno(strndup), vec_pop_f(run->filters, test_run_free_filter, NULL), -1;
        }
    }
    
    return 0;
}

static int test_run_check_traced(test_run_ct run)
{
    pid_t child, parent;
    int status, rc;
    
    if((child = fork()) < 0)
    {
        fperror(run->fp, "failed to check trace status");
        return error_set_errno(fork), -1;
    }
    else if(child)
    {
        if(waitpid(child, &status, 0) < 0)
        {
            fperror(run->fp, "failed to check trace status");
            return error_set_errno(waitpid), -1;
        }
        else if(WIFSIGNALED(status))
        {
            fprintf(run->fp, "failed to check trace status: %s\n", strsignal(WTERMSIG(status)));
            return error_set(E_TEST_RUN_TRACE_CHECK), -1;
        }
        else if((rc = WEXITSTATUS(status)) < 0)
        {
            fprintf(run->fp, "failed to check trace status: %s\n", strerror(-rc));
            return error_set(E_TEST_RUN_TRACE_CHECK), -1;
        }
        else switch(rc)
        {
        case TEST_CHILD_TRACED:        return TEST_PARENT_TRACED;
        case TEST_CHILD_NOT_TRACED:    return TEST_PARENT_NOT_TRACED;
        default:                       abort();
        }
    }
    else
    {
        parent = getppid();
        
        if(ptrace(PTRACE_ATTACH, parent, NULL, NULL) < 0)
        {
            rc = errno == EPERM ? TEST_CHILD_TRACED : -errno;
        }
        else
        {
            waitpid(parent, NULL, 0);
            
            rc = ptrace(PTRACE_DETACH, parent, NULL, NULL);
            rc = rc < 0 ? -errno : TEST_CHILD_NOT_TRACED;
        }
        
        if(run->clean_exit)
            return rc;
        
        _exit(rc);
    }
}

static int test_run_set_core_dump(test_run_ct run)
{
    struct rlimit limit;
    
    if(getrlimit(RLIMIT_CORE, &limit))
        return error_set_errno(getrlimit), fperror(run->fp, "failed to get core dump size"), -1;
    
    if(!run->core_dump && limit.rlim_cur)
    {
        limit.rlim_cur = 0;
        
        if(setrlimit(RLIMIT_CORE, &limit))
            return error_set_errno(setrlimit), fperror(run->fp, "failed to disable core dump"), -1;
    }
    else if(run->core_dump && !limit.rlim_cur)
    {
        limit.rlim_cur = limit.rlim_max;
        
        if(setrlimit(RLIMIT_CORE, &limit))
            return error_set_errno(setrlimit), fperror(run->fp, "failed to enable core dump"), -1;
    }
    
    return 0;
}

static bool test_run_match_filter_unit(vec_const_ct vec, const void *elem, void *ctx)
{
    const test_filter_unit_st *unit = elem;
    const char *name = ctx;
    size_t len;
    
    if(!unit->name)
        return true;
    
    len = strlen(name);
    
    if(len < unit->len
    || (len > unit->len && !unit->prefix)
    || strncmp(name, unit->name, unit->len))
        return false;
    
    return true;
}

static int test_run_match_filter(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    test_match_st *state = ctx;
    test_filter_st *filter = elem;
    test_filter_node_st *node;
    
    // filter did not match or filter path is prefix of test path
    if(filter->level < state->level
    || !(node = vec_at(filter->path, state->level)))
    {
        // last filter node matched -> keep matching
        if(filter->matched)
            state->matched++;
        
        return 0;
    }
    
    // test path is test case and filter path has unmatched nodes
    // or no unit matches test path
    if((state->type == TEST_ENTRY_CASE && vec_at(filter->path, state->level+1))
    || !vec_find(node->units, test_run_match_filter_unit, (void*)state->name))
    {
        filter->matched = false;
        return 0;
    }
    
    filter->matched = true;
    filter->level = state->level+1; // filter ready for next level
    state->matched++;
    
    return 0;
}

static bool test_run_filter(test_run_ct run, test_entry_id type, const char *name)
{
    test_match_st state = { .name = name, .type = type };
    
    if(!run->filters || vec_is_empty(run->filters))
        return true;
    
    state.level = run->path ? vec_size(run->path) : 0;
    
    vec_fold(run->filters, test_run_match_filter, &state);
    
    return state.matched > 0;
}

static int test_run_push(test_run_ct run, const char *name)
{
    if(!run->path && !(run->path = vec_new(10, sizeof(char*))))
        return error_push(), fperror(run->fp, "failed to init path"), -1;
    
    if(!vec_push_p(run->path, name))
        return error_push(), fperror(run->fp, "failed to push path"), -1;
    
    return 0;
}

static void test_run_pop(test_run_ct run)
{
    if(run->path)
    {
        vec_pop(run->path);
        run->path = vec_free_if_empty(run->path);
    }
}

static void test_run_print_path(test_run_ct run)
{
    size_t i;
    
    if(vec_is_empty(run->path))
        fprintf(run->fp, "<top>");
    else
        for(i=0; i < vec_size(run->path); i++)
            fprintf(run->fp, "%s%s", i ? "/" : "", (char*)vec_at_p(run->path, i));
}

static bool test_run_has_error(test_run_ct run)
{
    return run->count.results[TEST_RESULT_FAIL]
    ||     run->count.results[TEST_RESULT_TIMEOUT]
    ||     run->count.results[TEST_RESULT_ERROR];
}

static bool test_run_has_warning(test_run_ct run)
{
    return run->count.results[TEST_RESULT_WARNING]
    ||     run->count.results[TEST_RESULT_SKIP];
}

static size_t test_run_sum_count(test_run_ct run)
{
    size_t sum, c;
    
    for(sum=0, c=0; c < TEST_RESULTS; c++)
        sum += run->count.results[c];
    
    return sum;
}

static void test_run_add_count(test_run_ct run, test_count_st *count)
{
    size_t c;
    
    for(c=0; c < TEST_RESULTS; c++)
        run->count.results[c] += count->results[c];
    
    run->count.asserts += count->asserts;
}

static int test_run_entry(test_suite_const_ct suite, test_entry_st *entry, void *ctx);

static int test_run_suite(test_run_ct run, test_suite_const_ct suite)
{
    test_count_st count;
    int rc;
    
    if(!test_run_filter(run, TEST_ENTRY_SUITE, test_suite_get_name(suite)))
        return 0;
    
    memcpy(&count, &run->count, sizeof(test_count_st));
    memset(&run->count, 0, sizeof(test_count_st));
    
    if(test_run_push(run, test_suite_get_name(suite)))
        return error_propagate(), -1;
    
    if(run->loglvl >= TEST_LOGLVL_CASE)
    {
        test_run_print_path(run);
        fprintf(run->fp, "\n");
    }
    
    rc = test_suite_fold(suite, test_run_entry, run);
    
    if(run->loglvl >= TEST_LOGLVL_SUITE)
    {
        test_run_print_path(run);
        
        if(rc)
            fprintf(run->fp, COLOR_RED" error"COLOR_DEFAULT"\n");
        else
            fprintf(run->fp, ": %zu case%s %sdone"COLOR_DEFAULT"\n",
                test_run_sum_count(run), test_run_sum_count(run) == 1 ? "" : "s",
                test_run_has_error(run) ? COLOR_RED :
                test_run_has_warning(run) ? COLOR_YELLOW : COLOR_GREEN);
    }
    
    test_run_pop(run);
    
    test_run_add_count(run, &count);
    
    return rc;
}

static int test_run_worker(test_run_ct run, test_case_const_ct tcase)
{
    test_ctx_st ctx = { .com = run->com, .clock = run->worker_clock };
    test_case_cb fun;
    void *state = NULL;
    jmp_buf jump;
    int null;
    
    if(run->fork)
    {
        // put test process into own process group
        // so kill(0, SIGKILL) wont kill the runner
        setpgid(0, 0);
        
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
        if((null = open("/dev/null", O_RDWR)) >= 0)
        {
            dup(null);
            dup(null);
        }
    }
    else
    {
        ctx.jump = &jump;
        
        if(setjmp(jump)) // returns 1 on jump
            return 0;
    }
    
    proc_append_title(" [%s]", test_case_get_name(tcase));
    
    if((fun = test_case_get_setup(tcase)))
    {
        test_com_send_status(run->com, TEST_STATUS_SETUP);
        fun(&ctx, &state);
    }
    
    test_com_send_status(run->com, TEST_STATUS_RUN);
    clock_gettime(run->worker_clock, &ctx.start);
    
    fun = test_case_get_run(tcase);
    fun(&ctx, &state);
    
    test_com_send_duration(run->com, run->worker_clock, &ctx.start);
    
    if((fun = test_case_get_teardown(tcase)))
    {
        test_com_send_status(run->com, TEST_STATUS_TEARDOWN);
        fun(&ctx, &state);
    }
    
    test_com_send_status(run->com, TEST_STATUS_FINISH);
    
    if(run->fork && !run->clean_exit)
        _exit(0);
    
    // if forked return 1 to prevent child doing tests but cleanup properly
    return run->fork ? 1 : 0;
}

static int test_run_msg(test_com_msg_id type, test_com_msg_un *msg, void *ctx)
{
    test_run_ct run = ctx;
    
    switch(type)
    {
    case TEST_COM_STATUS:
        return test_state_set_status(run->state, msg->status);
    case TEST_COM_RESULT:
        return test_state_set_result(run->state, msg->result);
    case TEST_COM_DURATION:
        return test_state_add_duration(run->state, msg->duration);
    case TEST_COM_POS:
        return test_state_set_position(run->state, msg->pos.type, msg->pos.file, msg->pos.line);
    case TEST_COM_PASS:
        if(test_state_get_status(run->state) == TEST_STATUS_RUN)
            return test_state_inc_asserts(run->state);
        return 0;
    case TEST_COM_MSG:
        return test_state_add_msg(run->state, msg->msg.type, msg->msg.text);
    default:
        abort();
    }
}

static int test_run_print_msg(test_pos_st *pos, test_msg_id type, size_t level, char *msg, void *ctx)
{
    test_run_ct run = ctx;
    size_t l;
    
    for(l=0; l < level+2; l++)
        fprintf(run->fp, "  ");
    
    if(pos->file && !level)
    {
        fprintf(run->fp, "%s"COLOR_DEFAULT"%s:%zu\n",
            pos->type == TEST_POS_AFTER ? COLOR_YELLOW"after " : "",
            pos->file, pos->line);
        
        for(l=0; l < level+3; l++)
            fprintf(run->fp, "  ");
    }
    
    switch(type)
    {
    case TEST_MSG_INFO:     break;
    case TEST_MSG_WARNING:  fprintf(run->fp, COLOR_YELLOW); break;
    case TEST_MSG_ERROR:    fprintf(run->fp, COLOR_RED); break;
    default:                abort();
    }
    
    fprintf(run->fp, "%s"COLOR_DEFAULT"\n", msg);
    
    return 0;
}

static void test_run_eval(test_run_ct run, test_case_const_ct tcase)
{
    test_result_id result;
    size_t duration, asserts;
    char *color, *strduration;
    
    result = test_state_get_result(run->state);
    asserts = test_state_get_asserts(run->state);
    
    run->count.results[result]++;
    run->count.asserts += asserts;
    
    if(run->loglvl >= TEST_LOGLVL_CASE
    || (result != TEST_RESULT_PASS && result != TEST_RESULT_SKIP))
    {
        if(run->loglvl < TEST_LOGLVL_CASE)
        {
            test_run_print_path(run);
            fprintf(run->fp, "/%s: ", test_case_get_name(tcase));
        }
        
        switch(result)
        {
        case TEST_RESULT_PASS:      color = COLOR_GREEN; break;
        case TEST_RESULT_WARNING:
        case TEST_RESULT_SKIP:      color = COLOR_YELLOW; break;
        default:                    color = COLOR_RED; break;
        }
        
        fprintf(run->fp, "%zu assert%s %s%s"COLOR_DEFAULT,
            asserts, asserts == 1 ? "" : "s", color, test_state_strresult(result));
        
        if((duration = test_state_get_duration(run->state)))
        {
            strduration = time_strdup_duration(0, duration);
            fprintf(run->fp, " (%s)\n", strduration);
            free(strduration);
        }
        else
            fprintf(run->fp, "\n");
        
        test_state_fold_msg(run->state, test_run_print_msg, run);
    }
}

static void test_run_eval_status(test_run_ct run, test_case_const_ct tcase, bool signaled, int signal, bool exited, int rc)
{
    if(test_state_get_result(run->state) == TEST_RESULT_SKIP)
        ; // skip signal/rc inspection
    else if(signaled)
    {
        if(!test_case_expects_signal(tcase))
        {
            test_state_set_result(run->state, TEST_RESULT_ERROR);
            test_state_add_msg_f(run->state, TEST_MSG_ERROR,
                "signal %i (%s) during %s",
                signal, strsignal(signal), test_state_get_strstatus(run->state));
        }
        else if(signal != test_case_get_signal(tcase))
        {
            test_state_set_result(run->state, TEST_RESULT_FAIL);
            test_state_add_msg_f(run->state, TEST_MSG_ERROR,
                "signal %i (%s), expected %i (%s)",
                signal, strsignal(signal),
                test_case_get_signal(tcase), strsignal(test_case_get_signal(tcase)));
        }
    }
    else if(exited)
    {
        if(test_case_expects_signal(tcase))
        {
            test_state_set_result(run->state, TEST_RESULT_FAIL);
            test_state_add_msg_f(run->state, TEST_MSG_ERROR,
                "exit %i, expected signal %i (%s)",
                rc, test_case_get_signal(tcase), strsignal(test_case_get_signal(tcase)));
        }
        else if(test_case_expects_exit(tcase) && rc != test_case_get_exit(tcase))
        {
            test_state_set_result(run->state, TEST_RESULT_FAIL);
            test_state_add_msg_f(run->state, TEST_MSG_ERROR,
                "exit %i, expected %i",
                rc, test_case_get_exit(tcase));
        }
    }
    else
        abort();
    
    test_run_eval(run, tcase);
}

static int test_run_collect(test_run_ct run, test_case_const_ct tcase, pid_t worker)
{
    int status;
    
    if(waitpid(worker, &status, 0) < 0)
        return error_set_errno(waitpid), fperror(run->fp, "failed to wait for test worker"), -1;
    
    test_run_eval_status(run, tcase,
        WIFSIGNALED(status), WTERMSIG(status), WIFEXITED(status), WEXITSTATUS(status));
    
    return 0;
}

static int test_run_kill(test_run_ct run, pid_t worker)
{
    // kill whole process group
    if(killpg(worker, SIGKILL))
        return error_set_errno(killpg), fperror(run->fp, "failed to kill test worker"), -1;
    
    if(waitpid(worker, NULL, 0) < 0)
        return error_set_errno(waitpid), fperror(run->fp, "failed to wait for test worker"), -1;
    
    return 0;
}

static void tcperror(test_run_ct run, test_case_const_ct tcase, const char *msg)
{
    if(run->loglvl < TEST_LOGLVL_CASE)
        fprintf(run->fp, "%s:\n", test_case_get_name(tcase));
    else
        fprintf(run->fp, "\n");
    
    fperror(run->fp, msg);
}

static int test_run_control(test_run_ct run, test_case_const_ct tcase, pid_t worker)
{
    struct pollfd pfds[1];
    timespec_st start, end, timeout, diff;
    int rc, poll_timeout;
    
    pfds[0].fd = test_com_get_socket(run->com);
    pfds[0].events = POLLIN;
    
    if(!run->traced)
    {
        time_ts_set_sec(&timeout, test_case_get_timeout(tcase));
        clock_gettime(run->control_clock, &start);
    }
    
    while(1)
    {
        if(run->traced)
            poll_timeout = -1;
        else
            poll_timeout = MIN((size_t)INT_MAX, time_ts_get_milli(&timeout));
        
        if((rc = poll(pfds, 1, poll_timeout)) < 0)
            return error_set_errno(poll), tcperror(run, tcase, "failed to poll"), test_run_kill(run, worker), -1;
        
        if(!rc)
            break;
        
        if(pfds[0].revents & POLLIN
        && test_com_recv(run->com))
            return tcperror(run, tcase, "failed to recv"), test_run_kill(run, worker), -1;
        
        if(pfds[0].revents & POLLERR)
            return tcperror(run, tcase, "failed to poll"), test_run_kill(run, worker), -1;
        
        if(pfds[0].revents & POLLHUP)
            return test_run_collect(run, tcase, worker);
        
        if(!run->traced)
        {
            clock_gettime(run->control_clock, &end);
            time_ts_diff(&diff, &start, &end);
            
            if(time_ts_cmp(&diff, &timeout) >= 0)
                break;
            
            time_ts_diff(&timeout, &diff, &timeout);
            start = end;
        }
    }
    
    if(test_run_kill(run, worker))
        return error_propagate(), -1;
    
    test_state_set_result(run->state, TEST_RESULT_TIMEOUT);
    test_run_eval(run, tcase);
    
    return 0;
}

static int test_run_case(test_run_ct run, test_case_const_ct tcase)
{
    pid_t pid;
    int sv[2], rc = 0;
    
    if(!test_run_filter(run, TEST_ENTRY_CASE, test_case_get_name(tcase)))
        return 0;
    
    if(run->loglvl >= TEST_LOGLVL_CASE)
    {
        fprintf(run->fp, "  %s: ", test_case_get_name(tcase));
        fflush(run->fp);
    }
    
    if(!run->fork)
    {
        if(run->skip && (test_case_expects_exit(tcase) || test_case_expects_signal(tcase)))
            test_state_set_result(run->state, TEST_RESULT_SKIP);
        else
            rc = test_run_worker(run, tcase);
        
        if(rc >= 0)
            test_run_eval_status(run, tcase, false, 0, true, rc);
    }
    else
    {
        if(socketpair(AF_UNIX, SOCK_STREAM|SOCK_NONBLOCK, 0, sv) == -1)
            return error_set_errno(socketpair), tcperror(run, tcase, "failed to init com"), -1;
        
        if((pid = fork()) == -1)
        {
            error_set_errno(fork);
            tcperror(run, tcase, "failed to fork");
            return close(sv[0]), close(sv[1]), -1;
        }
        
        if(!pid)
        {
            close(sv[0]);
            test_com_set_socket(run->com, sv[1]);
            rc = test_run_worker(run, tcase);
        }
        else
        {
            close(sv[1]);
            test_com_set_socket(run->com, sv[0]);
            rc = test_run_control(run, tcase, pid);
        }
    }
    
    test_state_reset(run->state);
    test_com_reset(run->com);
    
    return rc;
}

static int test_run_entry(test_suite_const_ct suite, test_entry_st *entry, void *ctx)
{
    test_run_ct run = ctx;
    
    switch(entry->type)
    {
    case TEST_ENTRY_SUITE:  return test_run_suite(run, entry->value.suite);
    case TEST_ENTRY_CASE:   return test_run_case(run, entry->value.tcase);
    default:                abort();
    }
}

static void test_run_print_summary(test_run_ct run)
{
    test_result_id result;
    bool first = true;
    const char *color;
    
    fprintf(run->fp, "Summary: ");
    
    for(result=0; result < TEST_RESULTS; result++)
        if(run->count.results[result])
        {
            switch(result)
            {
            case TEST_RESULT_PASS:      color = COLOR_GREEN; break;
            case TEST_RESULT_WARNING:
            case TEST_RESULT_SKIP:      color = COLOR_YELLOW; break;
            default:                    color = COLOR_RED; break;
            }
            
            fprintf(run->fp, "%s%s%zu %s"COLOR_DEFAULT,
                first ? "" : ", ", color, run->count.results[result],
                test_state_strresult(result));
            
            first = false;
        }
    
    if(first)
        fprintf(run->fp, COLOR_YELLOW"no test cases executed"COLOR_DEFAULT);
    
    fprintf(run->fp, "\n");
}

int test_run_exec(test_run_ct run, test_suite_const_ct suite)
{
    int rc;
    
    return_error_if_fail(run, E_TEST_RUN_INVALID_OBJECT, -1);
    return_error_if_fail(suite, E_TEST_RUN_INVALID_SUITE, -1);
    
    if(run->fork && run->check_traced)
        switch((rc = test_run_check_traced(run)))
        {
        case TEST_PARENT_NOT_TRACED:
            break;
        case TEST_PARENT_TRACED:
            fprintf(run->fp, COLOR_YELLOW"tracer detected, disabling fork"COLOR_DEFAULT"\n");
            run->traced = true;
            run->fork = false;
            break;
        default:
            return rc;
        }
    
    test_com_enable_shortcut(run->com, !run->fork);
    
    test_run_set_core_dump(run);
    
    if(!(rc = test_run_suite(run, suite)))
        test_run_print_summary(run);
    
    return rc;
}
