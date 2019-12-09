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

#ifndef __YTIL_TEST_COM_H__
#define __YTIL_TEST_COM_H__

#include <ytil/test/state.h>
#include <ytil/ext/time.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>


typedef enum test_com_error
{
      E_TEST_COM_CALLBACK
    , E_TEST_COM_INVALID_MSG_TYPE
    , E_TEST_COM_INVALID_STATUS_TYPE
    , E_TEST_COM_INVALID_RESULT_TYPE
    , E_TEST_COM_INVALID_POS_TYPE
    , E_TEST_COM_INVALID_TIMESTAMP
    , E_TEST_COM_INVALID_TEXT_FORMAT
    , E_TEST_COM_NOT_AVAILABLE
    , E_TEST_COM_SHUTDOWN
    , E_TEST_COM_WOULD_BLOCK
} test_com_error_id;

typedef enum test_com_msg_type
{
      TEST_COM_STATUS
    , TEST_COM_RESULT
    , TEST_COM_DURATION
    , TEST_COM_POS
    , TEST_COM_PASS
    , TEST_COM_MSG
    , TEST_COM_TYPES
} test_com_msg_id;

typedef union test_com_msg
{
    test_status_id status;
    test_result_id result;
    size_t duration;
    test_pos_st pos;
    test_msg_st msg;
} test_com_msg_un;

struct test_com;
typedef struct test_com *test_com_ct;

typedef int (*test_com_msg_cb)(test_com_msg_id type, test_com_msg_un *msg, void *ctx);


test_com_ct test_com_new(test_com_msg_cb cb, void *ctx);
void        test_com_free(test_com_ct com);
void        test_com_reset(test_com_ct com);

void test_com_set_socket(test_com_ct com, int sock);
int  test_com_get_socket(test_com_ct com);

int test_com_enable_shortcut(test_com_ct com, bool shortcut);

int test_com_send_status(test_com_ct com, test_status_id status);
int test_com_send_result(test_com_ct com, test_result_id result);
int test_com_send_duration(test_com_ct com, clockid_t clock, timespec_st *start);
int test_com_send_position(test_com_ct com, test_pos_id type, const char *file, size_t line);
int test_com_send_pass(test_com_ct com);
int test_com_send_msg(test_com_ct com, test_msg_id type, size_t level, const char *fmt, ...) __attribute__((format (gnu_printf, 4, 5)));
int test_com_send_msg_v(test_com_ct com, test_msg_id type, size_t level, const char *fmt, va_list ap) __attribute__((format (gnu_printf, 4, 0)));

int test_com_recv(test_com_ct com);

#endif
