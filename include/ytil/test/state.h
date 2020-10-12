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

#ifndef YTIL_TEST_STATE_H_INCLUDED
#define YTIL_TEST_STATE_H_INCLUDED

#include <ytil/gen/error.h>
#include <stddef.h>

typedef enum test_state_error
{
      E_TEST_STATE_CALLBACK
    , E_TEST_STATE_INVALID_OBJECT
    , E_TEST_STATE_INVALID_STATUS_TYPE
    , E_TEST_STATE_INVALID_RESULT_TYPE
    , E_TEST_STATE_INVALID_FILE
    , E_TEST_STATE_INVALID_MSG
    , E_TEST_STATE_INVALID_MSG_TYPE
    , E_TEST_STATE_INVALID_CALLBACK
} test_state_error_id;

/// test_state error type declaration
ERROR_DECLARE(TEST_STATE);

typedef enum test_status
{
      TEST_STATUS_INIT
    , TEST_STATUS_SETUP
    , TEST_STATUS_RUN
    , TEST_STATUS_TEARDOWN
    , TEST_STATUS_FINISH
    , TEST_STATUSES
} test_status_id;

typedef enum test_result
{
      TEST_RESULT_PASS
    , TEST_RESULT_WARNING
    , TEST_RESULT_FAIL
    , TEST_RESULT_TIMEOUT
    , TEST_RESULT_ERROR
    , TEST_RESULT_SKIP
    , TEST_RESULTS
} test_result_id;

typedef enum test_pos_type
{
      TEST_POS_EXACT
    , TEST_POS_AFTER
    , TEST_POS_TYPES
} test_pos_id;

typedef struct test_pos
{
    test_pos_id type;
    char *file;
    size_t line;
} test_pos_st;

typedef enum test_msg_type
{
      TEST_MSG_INFO
    , TEST_MSG_WARNING
    , TEST_MSG_ERROR
    , TEST_MSG_TYPES
} test_msg_id;

typedef struct test_msg
{
    test_msg_id type;
    size_t level;
    char *text;
    test_pos_st pos;
} test_msg_st;

struct test_state;
typedef struct test_state *test_state_ct;

typedef int (*test_state_msg_cb)(test_pos_st *pos, test_msg_id type, size_t level, char *msg, void *ctx);

test_state_ct test_state_new(void);
void          test_state_free(test_state_ct state);
void          test_state_reset(test_state_ct state);

int test_state_set_status(test_state_ct state, test_status_id status);
int test_state_set_result(test_state_ct state, test_result_id result);
int test_state_add_duration(test_state_ct state, size_t duration);
int test_state_set_position(test_state_ct state, test_pos_id type, const char *file, size_t line);
int test_state_inc_asserts(test_state_ct state);

test_status_id  test_state_get_status(test_state_ct state);
const char     *test_state_get_strstatus(test_state_ct state);
test_result_id  test_state_get_result(test_state_ct state);
const char     *test_state_get_strresult(test_state_ct state);
size_t          test_state_get_duration(test_state_ct state);
test_pos_st    *test_state_get_position(test_state_ct state);
size_t          test_state_get_asserts(test_state_ct state);

const char *test_state_strstatus(test_status_id status);
const char *test_state_strresult(test_result_id result);

int test_state_add_msg(test_state_ct state, test_msg_id type, size_t level, const char *msg);
int test_state_add_msg_f(test_state_ct state, test_msg_id type, size_t level, const char *msg, ...) __attribute__((format (gnu_printf, 4, 5)));

int test_state_fold_msg(test_state_ct state, test_state_msg_cb fold, const void *ctx);

#endif
