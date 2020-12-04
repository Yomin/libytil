/*
 * Copyright (c) 2020 Martin Rödel a.k.a. Yomin Nimoy
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

#include "gen.h"
#include <ytil/test/run.h>
#include <ytil/test/test.h>
#include <ytil/gen/log.h>
#include <ytil/sys/path.h>
#include <ytil/sys/env.h>
#include <ytil/ext/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>


#define TESTFILE "ytil_test.log"

static ssize_t unit1, target1, unit2, target2;
static str_const_ct name;
static str_ct testfile;
static char *msg;


TEST_CASE_ABORT(log_unit_add_invalid_name1)
{
    log_unit_add(NULL);
}

TEST_CASE(log_unit_add_invalid_name2)
{
    test_int_error(log_unit_add(LIT("")), E_LOG_INVALID_NAME);
}

TEST_SETUP(log_unit_add)
{
    path_ct path;

    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_ptr_success(path_append_c(path, TESTFILE, PATH_STYLE_POSIX));
    test_ptr_success(testfile = path_get(path, PATH_STYLE_NATIVE));
    test_void(path_free(path));
    test_void(env_free());

    test_int_success(unit1 = log_unit_add(LIT("test")));
}

TEST_TEARDOWN(log_free)
{
    test_void(log_free());

    if(testfile)
    {
        str_unref(testfile);
        testfile = NULL;
    }
}

TEST_CASE_FIX(log_unit_add, log_unit_add, log_free)
{
    test_uint_eq(log_units(), 1);
}

TEST_CASE_FIX(log_unit_add_existing, log_unit_add, log_free)
{
    test_int_error(log_unit_add(LIT("test")), E_LOG_EXISTS);
}

TEST_CASE_ABORT(log_unit_get_invalid_name)
{
    log_unit_get(NULL, true);
}

TEST_CASE_FIX(log_unit_get_not_found, log_unit_add, log_free)
{
    test_int_error(log_unit_get(LIT("t"), true), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_unit_get, log_unit_add, log_free)
{
    test_int_success(log_unit_get(LIT("test"), true));
}

TEST_CASE_FIX(log_unit_get_with_prefix, log_unit_add, log_free)
{
    test_int_success(log_unit_get(LIT("t"), false));
}

TEST_CASE_FIX(log_unit_get_name_not_found1, log_unit_add, log_free)
{
    test_ptr_error(log_unit_get_name(0), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_unit_get_name_not_found2, log_unit_add, log_free)
{
    test_ptr_error(log_unit_get_name(123), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_unit_get_name, log_unit_add, log_free)
{
    test_ptr_success(name = log_unit_get_name(unit1));
    test_str_eq(str_c(name), "test");
}

TEST_CASE_FIX(log_unit_get_max_level_not_found1, log_unit_add, log_free)
{
    test_uint_eq(log_unit_get_max_level(0), LOG_INVALID);
    test_error(0, LOG, E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_unit_get_max_level_not_found2, log_unit_add, log_free)
{
    test_uint_eq(log_unit_get_max_level(123), LOG_INVALID);
    test_error(0, LOG, E_LOG_NOT_FOUND);
}

TEST_CASE_ABORT(log_unit_fold_invalid_callback)
{
    log_unit_fold(NULL, NULL);
}

static int test_log_fold_unit(size_t id, str_const_ct name, void *ctx)
{
    size_t *sum = ctx;

    *sum += id;

    return 0;
}

TEST_CASE_FIX(log_unit_fold, no_setup, log_free)
{
    size_t sum = 0;

    test_int_success(log_unit_add(LIT("foo")));
    test_int_success(log_unit_add(LIT("bar")));
    test_int_success(log_unit_add(LIT("baz")));

    test_int_success(log_unit_fold(test_log_fold_unit, &sum));
    test_int_eq(sum, 1 + 2 + 3);
}

#ifndef O_BINARY
    #define O_BINARY 0
#endif

static char *test_log_read_file(const char *file)
{
    int fd;
    off_t size;
    char *str, *ptr;
    ssize_t count;

    log_flush();

    if((fd = open(file, O_RDONLY | O_BINARY)) < 0)
        return NULL;

    if((size = lseek(fd, 0, SEEK_END)) == (off_t)-1)
        return close(fd), NULL;

    if(lseek(fd, 0, SEEK_SET) == (off_t)-1)
        return close(fd), NULL;

    if(!(str = calloc(1, size + 1)))
        return close(fd), NULL;

    for(ptr = str; size; size -= count, ptr += count)
        if((count = read(fd, ptr, size)) < 0)
            return free(str), close(fd), NULL;
        else if(!count)
            break;

    close(fd);

    return str;
}

TEST_SETUP(path_new)
{
    path_ct path;

    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_ptr_success(path_append_c(path, TESTFILE, PATH_STYLE_POSIX));
    test_ptr_success(testfile = path_get(path, PATH_STYLE_NATIVE));
    test_void(path_free(path));
    test_void(env_free());
}

TEST_TEARDOWN(path_free)
{
    test_void(str_unref(testfile));
    testfile = NULL;
}

TEST_TEARDOWN(log_free_unlink)
{
    test_void(log_free());
    test_int_maybe_errno(unlink(str_c(testfile)), ENOENT);

    if(testfile)
    {
        str_unref(testfile);
        testfile = NULL;
    }

    if(msg)
    {
        free(msg);
        msg = NULL;
    }
}

TEST_CASE_FIX(log_target_add_file_invalid_name, path_new, path_free)
{
    test_int_error(log_target_add_file(LIT(""), testfile, false, LOG_COLOR_OFF), E_LOG_INVALID_NAME);
}

TEST_CASE_ABORT(log_target_add_file_invalid_file1)
{
    log_target_add_file(LIT("foo"), NULL, false, LOG_COLOR_OFF);
}

TEST_CASE(log_target_add_file_invalid_file2)
{
    test_int_error(log_target_add_file(LIT("foo"), LIT("/"), false, LOG_COLOR_OFF), E_LOG_FOPEN);
}

TEST_CASE_FIX_ABORT(log_target_add_file_invalid_color, path_new, path_free)
{
    log_target_add_file(LIT("foo"), testfile, false, 123);
}

TEST_CASE_FIX(log_target_add_file_named, path_new, log_free_unlink)
{
    test_int_success(target1 =
            log_target_add_file(LIT("foo"), testfile, false, LOG_COLOR_OFF));

    test_uint_eq(log_targets(), 1);
    test_ptr_success(name = log_target_get_name(target1));
    test_str_eq(str_c(name), "foo");
}

TEST_CASE_FIX(log_target_add_file_unnamed, path_new, log_free_unlink)
{
    test_int_success(target1 =
            log_target_add_file(NULL, testfile, false, LOG_COLOR_OFF));

    test_uint_eq(log_targets(), 1);
    test_ptr_success(name = log_target_get_name(target1));
    test_str_eq(str_c(name), str_c(testfile));
}

TEST_CASE_FIX(log_target_add_file, log_unit_add, log_free_unlink)
{
    test_int_success(target1 = log_target_add_file(NULL, testfile, false, LOG_COLOR_OFF));
    test_int_success(log_sink_set_level(unit1, target1, LOG_INFO));

    test_int_success(log_info(unit1, "foo"));
    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, "foo\n");
}

TEST_CASE_ABORT(log_target_add_stream_invalid_name1)
{
    log_target_add_stream(NULL, stdout, false, LOG_COLOR_OFF);
}

TEST_CASE(log_target_add_stream_invalid_name2)
{
    test_int_error(log_target_add_stream(LIT(""), stdout, false, LOG_COLOR_OFF),
        E_LOG_INVALID_NAME);
}

TEST_CASE_ABORT(log_target_add_stream_invalid_stream)
{
    test_int_error(log_target_add_stream(LIT("foo"), NULL, false, LOG_COLOR_OFF),
        E_LOG_INVALID_STREAM);
}

TEST_CASE_ABORT(log_target_add_stream_invalid_color)
{
    log_target_add_stream(LIT("foo"), stdout, false, 123);
}

#ifndef _WIN32
static int test_log_intercept(FILE *stream, const char *file)
{
    static int fd_original;
    int fd_file, fd = fileno(stream);

    fflush(stream);

    if(file)
    {
        if((fd_original = dup(fd)) < 0)
            return -1;

        if((fd_file = open(file, O_CREAT | O_WRONLY, 0664)) < 0)
            return close(fd_original), -1;

        if(dup2(fd_file, fd) < 0)
            return close(fd_original), close(fd_file), -1;

        close(fd_file);
    }
    else
    {
        dup2(fd_original, fd);
        close(fd_original);
    }

    return 0;
}
#endif /* ifndef _WIN32 */

TEST_CASE_FIX(log_target_add_stream, log_unit_add, log_free_unlink)
{
    test_int_success(target1 =
            log_target_add_stream(LIT("foo"), stdout, false, LOG_COLOR_OFF));

    test_uint_eq(log_targets(), 1);
    test_ptr_success(name = log_target_get_name(target1));
    test_str_eq(str_c(name), "foo");

#ifndef _WIN32
    test_int_success(log_sink_set_level(unit1, target1, LOG_INFO));
    test_int_success_errno(test_log_intercept(stdout, str_c(testfile)));

    test_int_success(log_info(unit1, "foo"));
    test_log_intercept(stdout, NULL);

    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, "foo\n");
#endif
}

TEST_CASE_ABORT(log_target_add_stdout_invalid_color)
{
    log_target_add_stdout(123);
}

TEST_CASE_FIX(log_target_add_stdout, log_unit_add, log_free_unlink)
{
    test_int_success(log_target_add_stdout(LOG_COLOR_OFF));
    test_uint_eq(log_targets(), 1);

#ifndef _WIN32
    test_int_success(log_sink_set_level(unit1, target1, LOG_INFO));
    test_int_success_errno(test_log_intercept(stdout, str_c(testfile)));

    test_int_success(log_info(unit1, "foo"));
    test_log_intercept(stdout, NULL);

    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, "foo\n");
#endif
}

TEST_CASE_ABORT(log_target_add_stderr_invalid_color)
{
    log_target_add_stderr(123);
}

TEST_CASE_FIX(log_target_add_stderr, log_unit_add, log_free_unlink)
{
    test_int_success(log_target_add_stderr(LOG_COLOR_OFF));
    test_uint_eq(log_targets(), 1);

#ifndef _WIN32
    test_int_success(log_sink_set_level(unit1, target1, LOG_INFO));
    test_int_success_errno(test_log_intercept(stderr, str_c(testfile)));

    test_int_success(log_info(unit1, "foo"));
    test_log_intercept(stderr, NULL);

    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, "foo\n");
#endif
}

TEST_SETUP(log_target_add)
{
    test_int_success(target1 = log_target_add_stdout(LOG_COLOR_OFF));
}

TEST_SETUP(log_unit_target_add)
{
    test_int_success(unit1      = log_unit_add(LIT("test1")));
    test_int_success(unit2      = log_unit_add(LIT("test2")));
    test_int_success(target1    = log_target_add_stdout(LOG_COLOR_OFF));
    test_int_success(target2    = log_target_add_stderr(LOG_COLOR_OFF));
}

TEST_CASE_ABORT(log_target_get_invalid_name)
{
    log_target_get(NULL, true);
}

TEST_CASE_FIX(log_target_get_not_found, log_target_add, log_free)
{
    test_int_error(log_target_get(LIT("std"), true), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_target_get, log_target_add, log_free)
{
    test_int_success(log_target_get(LIT("stdout"), true));
}

TEST_CASE_FIX(log_target_get_with_prefix, log_target_add, log_free)
{
    test_int_success(log_target_get(LIT("std"), false));
}

TEST_CASE_FIX(log_target_get_name_not_found1, log_target_add, log_free)
{
    test_ptr_error(log_target_get_name(0), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_target_get_name_not_found2, log_target_add, log_free)
{
    test_ptr_error(log_target_get_name(123), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_target_remove_not_found1, log_target_add, log_free)
{
    test_int_error(log_target_remove(0), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_target_remove_not_found2, log_target_add, log_free)
{
    test_int_error(log_target_remove(123), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_target_remove, log_target_add, log_free)
{
    test_int_success(log_target_remove(target1));
    test_uint_eq(log_targets(), 0);
}

TEST_CASE_FIX(log_target_remove_sinks, log_unit_target_add, log_free)
{
    test_int_success(log_sink_set_level(LOG_ALL_UNITS, LOG_ALL_TARGETS, LOG_INFO));
    test_int_success(log_target_remove(target1));

    test_uint_eq(log_targets(), 1);
    test_int_eq(log_sinks(unit1), 1);
    test_int_eq(log_sinks(unit2), 1);
}

TEST_CASE_FIX(log_target_set_hook_not_found1, log_target_add, log_free)
{
    test_int_error(log_target_set_hook(0, NULL, NULL), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_target_set_hook_not_found2, log_target_add, log_free)
{
    test_int_error(log_target_set_hook(123, NULL, NULL), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_target_unset_hook, log_target_add, log_free)
{
    test_int_success(log_target_set_hook(target1, NULL, NULL));
}

typedef struct test_log_hook_state
{
    char        *msg_start, *msg_end;
    const char  *file;
} test_log_hook_st;

static void test_log_hook(size_t id, str_const_ct name, bool start, void *ctx)
{
    test_log_hook_st *state = ctx;

    if(start)
        state->msg_start = test_log_read_file(state->file);
    else
        state->msg_end = test_log_read_file(state->file);
}

TEST_SETUP(log_init)
{
    path_ct path;

    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_ptr_success(path_append_c(path, TESTFILE, PATH_STYLE_POSIX));
    test_ptr_success(testfile = path_get(path, PATH_STYLE_NATIVE));
    test_void(path_free(path));
    test_void(env_free());

    test_int_success(unit1      = log_unit_add(LIT("test")));
    test_int_success(target1    = log_target_add_file(NULL, testfile, false, LOG_COLOR_OFF));
    test_int_success(log_sink_set_level(unit1, target1, LOG_INFO));
    msg = NULL;
}

TEST_CASE_FIX(log_target_set_hook, log_init, log_free_unlink)
{
    test_log_hook_st state = { NULL, NULL, str_c(testfile) };

    test_int_success(log_target_set_hook(target1, test_log_hook, &state));
    test_int_success(log_crit(unit1, "foo"));

    test_str_eq(state.msg_start, "");
    test_str_eq(state.msg_end, "foo\n");

    free(state.msg_start);
    free(state.msg_end);
}

TEST_CASE_ABORT(log_target_fold_invalid_callback)
{
    log_target_fold(NULL, NULL);
}

static int test_log_fold_target(size_t id, str_const_ct name, void *ctx)
{
    size_t *sum = ctx;

    *sum += id;

    return 0;
}

TEST_CASE_FIX(log_target_fold, no_setup, log_free)
{
    size_t sum = 0;

    test_int_success(log_target_add_stdout(LOG_COLOR_OFF));
    test_int_success(log_target_add_stdout(LOG_COLOR_OFF));
    test_int_success(log_target_add_stdout(LOG_COLOR_OFF));

    test_int_success(log_target_fold(test_log_fold_target, &sum));
    test_int_eq(sum, 1 + 2 + 3);
}

TEST_CASE_FIX(log_sink_set_level_not_found1, log_unit_target_add, log_free)
{
    test_int_error(log_sink_set_level(123, target1, LOG_OFF), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_sink_set_level_not_found2, log_unit_target_add, log_free)
{
    test_int_error(log_sink_set_level(unit1, 123, LOG_OFF), E_LOG_NOT_FOUND);
}

TEST_CASE_ABORT(log_sink_set_level_invalid_level1)
{
    log_sink_set_level(0, 0, LOG_INVALID);
}

TEST_CASE_ABORT(log_sink_set_level_invalid_level2)
{
    log_sink_set_level(0, 0, LOG_LEVELS);
}

TEST_CASE_FIX(log_sink_set_level, log_unit_target_add, log_free)
{
    test_int_success(log_sink_set_level(unit1, target1, LOG_INFO));

    test_int_eq(log_sinks(unit1), 1);
    test_int_eq(log_sinks(unit2), 0);
    test_uint_eq(log_sink_get_level(unit1, target1), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit1, target2), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit2, target1), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit2, target2), LOG_OFF);
}

TEST_CASE_FIX(log_sink_set_level_all_units, log_unit_target_add, log_free)
{
    test_int_success(log_sink_set_level(LOG_ALL_UNITS, target1, LOG_INFO));

    test_int_eq(log_sinks(unit1), 1);
    test_int_eq(log_sinks(unit2), 1);
    test_uint_eq(log_sink_get_level(unit1, target1), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit1, target2), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit2, target1), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit2, target2), LOG_OFF);
}

TEST_CASE_FIX(log_sink_set_level_all_targets, log_unit_target_add, log_free)
{
    test_int_success(log_sink_set_level(unit1, LOG_ALL_TARGETS, LOG_INFO));

    test_int_eq(log_sinks(unit1), 2);
    test_int_eq(log_sinks(unit2), 0);
    test_uint_eq(log_sink_get_level(unit1, target1), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit1, target2), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit2, target1), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit2, target2), LOG_OFF);
}

TEST_CASE_FIX(log_sink_set_level_all, log_unit_target_add, log_free)
{
    test_int_success(log_sink_set_level(LOG_ALL_UNITS, LOG_ALL_TARGETS, LOG_INFO));

    test_int_eq(log_sinks(unit1), 2);
    test_int_eq(log_sinks(unit2), 2);
    test_uint_eq(log_sink_get_level(unit1, target1), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit1, target2), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit2, target1), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit2, target2), LOG_INFO);
}

TEST_CASE_FIX(log_sink_set_level_off, log_unit_target_add, log_free)
{
    test_int_success(log_sink_set_level(LOG_ALL_UNITS, LOG_ALL_TARGETS, LOG_INFO));
    test_int_success(log_sink_set_level(unit1, target1, LOG_OFF));

    test_int_eq(log_sinks(unit1), 1);
    test_int_eq(log_sinks(unit2), 2);
    test_uint_eq(log_sink_get_level(unit1, target1), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit1, target2), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit2, target1), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit2, target2), LOG_INFO);
}

TEST_CASE_FIX(log_sink_set_level_off_all_units, log_unit_target_add, log_free)
{
    test_int_success(log_sink_set_level(LOG_ALL_UNITS, LOG_ALL_TARGETS, LOG_INFO));
    test_int_success(log_sink_set_level(LOG_ALL_UNITS, target1, LOG_OFF));

    test_int_eq(log_sinks(unit1), 1);
    test_int_eq(log_sinks(unit2), 1);
    test_uint_eq(log_sink_get_level(unit1, target1), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit1, target2), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit2, target1), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit2, target2), LOG_INFO);
}

TEST_CASE_FIX(log_sink_set_level_off_all_targets, log_unit_target_add, log_free)
{
    test_int_success(log_sink_set_level(LOG_ALL_UNITS, LOG_ALL_TARGETS, LOG_INFO));
    test_int_success(log_sink_set_level(unit1, LOG_ALL_TARGETS, LOG_OFF));

    test_int_eq(log_sinks(unit1), 0);
    test_int_eq(log_sinks(unit2), 2);
    test_uint_eq(log_sink_get_level(unit1, target1), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit1, target2), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit2, target1), LOG_INFO);
    test_uint_eq(log_sink_get_level(unit2, target2), LOG_INFO);
}

TEST_CASE_FIX(log_sink_set_level_off_all, log_unit_target_add, log_free)
{
    test_int_success(log_sink_set_level(LOG_ALL_UNITS, LOG_ALL_TARGETS, LOG_INFO));
    test_int_success(log_sink_set_level(LOG_ALL_UNITS, LOG_ALL_TARGETS, LOG_OFF));

    test_int_eq(log_sinks(unit1), 0);
    test_int_eq(log_sinks(unit2), 0);
    test_uint_eq(log_sink_get_level(unit1, target1), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit1, target2), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit2, target1), LOG_OFF);
    test_uint_eq(log_sink_get_level(unit2, target2), LOG_OFF);
}

TEST_CASE_FIX(log_sink_get_level_not_found1, log_unit_target_add, log_free)
{
    test_uint_eq(log_sink_get_level(0, target1), LOG_INVALID);
    test_error(0, LOG, E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_sink_get_level_not_found2, log_unit_target_add, log_free)
{
    test_uint_eq(log_sink_get_level(123, target1), LOG_INVALID);
    test_error(0, LOG, E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_sink_get_level_not_found3, log_unit_target_add, log_free)
{
    test_uint_eq(log_sink_get_level(unit1, 0), LOG_INVALID);
    test_error(0, LOG, E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_sink_get_level_not_found4, log_unit_target_add, log_free)
{
    test_uint_eq(log_sink_get_level(unit1, 123), LOG_INVALID);
    test_error(0, LOG, E_LOG_NOT_FOUND);
}

TEST_CASE_ABORT(log_sink_fold_invalid_callback)
{
    log_sink_fold(0, NULL, NULL);
}

static int test_log_fold_sink(size_t unit, str_const_ct unit_name, size_t target, str_const_ct target_name, log_level_id level, void *ctx)
{
    size_t *sum = ctx;

    *sum += unit + target + level;

    return 0;
}

TEST_CASE_FIX(log_sink_fold_not_found1, log_unit_add, log_free)
{
    test_int_error(log_sink_fold(0, test_log_fold_sink, NULL), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_sink_fold_not_found2, log_unit_add, log_free)
{
    test_int_error(log_sink_fold(123, test_log_fold_sink, NULL), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_sink_fold, log_unit_target_add, log_free)
{
    size_t sum = 0;

    test_int_success(log_sink_set_level(LOG_ALL_UNITS, target1, LOG_INFO));
    test_int_success(log_sink_set_level(LOG_ALL_UNITS, target2, LOG_TRACE));

    test_int_success(log_sink_fold(unit1, test_log_fold_sink, &sum));
    test_int_eq(sum, (1 + 1 + LOG_INFO) + (1 + 2 + LOG_TRACE));
}

TEST_CASE_FIX(log_sinks_not_found1, log_unit_target_add, log_free)
{
    test_int_error(log_sinks(0), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_sinks_not_found2, log_unit_target_add, log_free)
{
    test_int_error(log_sinks(123), E_LOG_NOT_FOUND);
}

TEST_CASE(log_level_get_not_found)
{
    test_uint_eq(log_level_get(LIT("i"), true), LOG_INVALID);
    test_error(0, LOG, E_LOG_NOT_FOUND);
}

TEST_CASE(log_level_get)
{
    test_uint_eq(log_level_get(LIT("info"), true), LOG_INFO);
}

TEST_CASE(log_level_get_with_prefix)
{
    test_uint_eq(log_level_get(LIT("i"), false), LOG_INFO);
}

TEST_CASE_ABORT(log_level_get_name_invalid_level1)
{
    log_level_get_name(LOG_INVALID);
}

TEST_CASE_ABORT(log_level_get_name_invalid_level2)
{
    log_level_get_name(LOG_LEVELS);
}

TEST_CASE(log_level_get_name)
{
    test_str_eq(log_level_get_name(LOG_INFO), "info");
}

TEST_CASE(log_prefix_unset)
{
    test_int_success(log_prefix_set(NULL));
}

TEST_CASE_FIX(log_prefix_set_pad_none, log_init, log_free_unlink)
{
    char test_msg[200];
    time_t now = time(NULL);

    snprintf(test_msg, sizeof(test_msg),
        "[%s] [%s] [%ld] [%s] [%s] [%s]: foo\n",
        "^", "INFO", (long)getpid(), str_c(testfile), "test", time_isodate(localtime(&now)));

    test_int_success(log_prefix_set(LIT("[^^] [^l] [^p] [^t] [^u] [^D]: ")));
    test_int_success(log_info(unit1, "foo"));
    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, test_msg);
}

TEST_CASE_FIX(log_prefix_set_pad_left, log_init, log_free_unlink)
{
    char test_msg[200];
    time_t now = time(NULL);

    snprintf(test_msg, sizeof(test_msg),
        "[%10s] [%10s] [%10ld] [%25s] [%10s] [%15s]: foo\n",
        "^", "INFO", (long)getpid(), str_c(testfile), "test", time_isodate(localtime(&now)));

    test_int_success(log_prefix_set(LIT("[^10^] [^10l] [^10p] [^25t] [^10u] [^15D]: ")));
    test_int_success(log_info(unit1, "foo"));
    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, test_msg);
}

TEST_CASE_FIX(log_prefix_set_pad_right, log_init, log_free_unlink)
{
    char test_msg[200];
    time_t now = time(NULL);

    snprintf(test_msg, sizeof(test_msg),
        "[%-10s] [%-10s] [%-10ld] [%-25s] [%-10s] [%-15s]: foo\n",
        "^", "INFO", (long)getpid(), str_c(testfile), "test", time_isodate(localtime(&now)));

    test_int_success(log_prefix_set(LIT("[^-10^] [^-10l] [^-10p] [^-25t] [^-10u] [^-15D]: ")));
    test_int_success(log_info(unit1, "foo"));
    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, test_msg);
}

TEST_CASE_FIX(log_msg_not_found1, log_unit_add, log_free)
{
    test_int_error(log_msg(0, LOG_INFO, "foo"), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_msg_not_found2, log_unit_add, log_free)
{
    test_int_error(log_msg(123, LOG_INFO, "foo"), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX_ABORT(log_msg_invalid_level1, log_unit_add, log_free)
{
    log_msg(unit1, LOG_INVALID, "foo");
}

TEST_CASE_FIX_ABORT(log_msg_invalid_level2, log_unit_add, log_free)
{
    log_msg(unit1, LOG_LEVELS, "foo");
}

TEST_CASE_FIX_ABORT(log_msg_invalid_msg, log_unit_add, log_free)
{
    log_msg(unit1, LOG_INFO, NULL);
}

TEST_CASE_FIX(log_msg_level_lt, log_init, log_free_unlink)
{
    test_int_success(log_msg(unit1, LOG_CRIT, "foo"));
    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, "foo\n");
}

TEST_CASE_FIX(log_msg_level_eq, log_init, log_free_unlink)
{
    test_int_success(log_msg(unit1, LOG_INFO, "foo"));
    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, "foo\n");
}

TEST_CASE_FIX(log_msg_level_gt, log_init, log_free_unlink)
{
    test_int_success(log_msg(unit1, LOG_DEBUG, "foo"));
    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, "");
}

TEST_CASE_FIX(log_msg_e_not_found1, log_unit_add, log_free)
{
    test_int_error(log_msg_e(0, LOG_INFO, "foo"), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX(log_msg_e_not_found2, log_unit_add, log_free)
{
    test_int_error(log_msg_e(123, LOG_INFO, "foo"), E_LOG_NOT_FOUND);
}

TEST_CASE_FIX_ABORT(log_msg_e_invalid_level1, log_unit_add, log_free)
{
    log_msg_e(unit1, LOG_INVALID, "foo");
}

TEST_CASE_FIX_ABORT(log_msg_e_invalid_level2, log_unit_add, log_free)
{
    log_msg_e(unit1, LOG_LEVELS, "foo");
}

TEST_CASE_FIX_ABORT(log_msg_e_invalid_msg, log_unit_add, log_free)
{
    log_msg_e(unit1, LOG_INFO, NULL);
}

TEST_CASE_FIX(log_msg_e_level_lt, log_init, log_free_unlink)
{
    char test_msg[200];

    error_set_s(ERRNO, E2BIG);
    snprintf(test_msg, sizeof(test_msg), "foo: %s\n", error_desc(0));

    test_int_success((error_set_s(ERRNO, E2BIG), log_msg_e(unit1, LOG_CRIT, "foo")));
    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, test_msg);
}

TEST_CASE_FIX(log_msg_e_level_eq, log_init, log_free_unlink)
{
    char test_msg[200];

    error_set_s(ERRNO, E2BIG);
    snprintf(test_msg, sizeof(test_msg), "foo: %s\n", error_desc(0));

    test_int_success((error_set_s(ERRNO, E2BIG), log_msg_e(unit1, LOG_INFO, "foo")));
    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, test_msg);
}

TEST_CASE_FIX(log_msg_e_level_gt, log_init, log_free_unlink)
{
    test_int_success((error_set_s(ERRNO, E2BIG), log_msg_e(unit1, LOG_DEBUG, "foo")));
    test_ptr_success_errno(msg = test_log_read_file(str_c(testfile)));
    test_str_eq(msg, "");
}

int test_suite_gen_log(void)
{
    return error_pass_int(test_run_cases("log",
        test_case(log_unit_add_invalid_name1),
        test_case(log_unit_add_invalid_name2),
        test_case(log_unit_add),
        test_case(log_unit_add_existing),

        test_case(log_unit_get_invalid_name),
        test_case(log_unit_get_not_found),
        test_case(log_unit_get),
        test_case(log_unit_get_with_prefix),

        test_case(log_unit_get_name_not_found1),
        test_case(log_unit_get_name_not_found2),
        test_case(log_unit_get_name),

        test_case(log_unit_get_max_level_not_found1),
        test_case(log_unit_get_max_level_not_found2),

        test_case(log_unit_fold_invalid_callback),
        test_case(log_unit_fold),

        test_case(log_target_add_file_invalid_name),
        test_case(log_target_add_file_invalid_file1),
        test_case(log_target_add_file_invalid_file2),
        test_case(log_target_add_file_invalid_color),
        test_case(log_target_add_file_named),
        test_case(log_target_add_file_unnamed),
        test_case(log_target_add_file),

        test_case(log_target_add_stream_invalid_name1),
        test_case(log_target_add_stream_invalid_name2),
        test_case(log_target_add_stream_invalid_stream),
        test_case(log_target_add_stream_invalid_color),
        test_case(log_target_add_stream),

        test_case(log_target_add_stdout_invalid_color),
        test_case(log_target_add_stdout),
        test_case(log_target_add_stderr_invalid_color),
        test_case(log_target_add_stderr),

        test_case(log_target_get_invalid_name),
        test_case(log_target_get_not_found),
        test_case(log_target_get),
        test_case(log_target_get_with_prefix),

        test_case(log_target_get_name_not_found1),
        test_case(log_target_get_name_not_found2),

        test_case(log_target_remove_not_found1),
        test_case(log_target_remove_not_found2),
        test_case(log_target_remove),
        test_case(log_target_remove_sinks),

        test_case(log_target_set_hook_not_found1),
        test_case(log_target_set_hook_not_found2),
        test_case(log_target_unset_hook),
        test_case(log_target_set_hook),

        test_case(log_target_fold_invalid_callback),
        test_case(log_target_fold),

        test_case(log_sink_set_level_not_found1),
        test_case(log_sink_set_level_not_found2),
        test_case(log_sink_set_level_invalid_level1),
        test_case(log_sink_set_level_invalid_level2),
        test_case(log_sink_set_level),
        test_case(log_sink_set_level_all_units),
        test_case(log_sink_set_level_all_targets),
        test_case(log_sink_set_level_all),
        test_case(log_sink_set_level_off),
        test_case(log_sink_set_level_off_all_units),
        test_case(log_sink_set_level_off_all_targets),
        test_case(log_sink_set_level_off_all),

        test_case(log_sink_get_level_not_found1),
        test_case(log_sink_get_level_not_found2),
        test_case(log_sink_get_level_not_found3),
        test_case(log_sink_get_level_not_found4),

        test_case(log_sink_fold_invalid_callback),
        test_case(log_sink_fold_not_found1),
        test_case(log_sink_fold_not_found2),
        test_case(log_sink_fold),

        test_case(log_sinks_not_found1),
        test_case(log_sinks_not_found2),

        test_case(log_level_get_not_found),
        test_case(log_level_get),
        test_case(log_level_get_with_prefix),

        test_case(log_level_get_name_invalid_level1),
        test_case(log_level_get_name_invalid_level2),
        test_case(log_level_get_name),

        test_case(log_prefix_unset),
        test_case(log_prefix_set_pad_none),
        test_case(log_prefix_set_pad_left),
        test_case(log_prefix_set_pad_right),

        test_case(log_msg_not_found1),
        test_case(log_msg_not_found2),
        test_case(log_msg_invalid_level1),
        test_case(log_msg_invalid_level2),
        test_case(log_msg_invalid_msg),
        test_case(log_msg_level_lt),
        test_case(log_msg_level_eq),
        test_case(log_msg_level_gt),

        test_case(log_msg_e_not_found1),
        test_case(log_msg_e_not_found2),
        test_case(log_msg_e_invalid_level1),
        test_case(log_msg_e_invalid_level2),
        test_case(log_msg_e_invalid_msg),
        test_case(log_msg_e_level_lt),
        test_case(log_msg_e_level_eq),
        test_case(log_msg_e_level_gt),

        NULL
    ));
}
