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

#ifndef YTIL_TEST_COM_H_INCLUDED
#define YTIL_TEST_COM_H_INCLUDED

#include <ytil/test/case.h>
#include <stddef.h>
#include <stdarg.h>


/// test com position data
typedef struct test_com_pos
{
    char    *file;      ///< file
    size_t  line;       ///< line in file
    bool    after;      ///< position is after line
} test_com_pos_st;

/// test com add-message data
typedef struct test_com_msg_add
{
    test_msg_id type;   ///< message type
    char        *msg;   ///< message text
} test_com_msg_add_st;

/// test com append-message data
typedef struct test_com_msg_append
{
    int     level;      ///< message level
    char    *msg;       ///< message text
} test_com_msg_append_st;

/// test com push-call data
typedef struct test_com_call_push
{
    char *call;         ///< call text
} test_com_call_push_st;

/// test com data type
typedef enum test_com_data_type
{
    TEST_COM_POS,           ///< position message
    TEST_COM_MSG_ADD,       ///< add text message
    TEST_COM_MSG_APPEND,    ///< append text message
    TEST_COM_CALL_PUSH,     ///< push call message
    TEST_COM_CALL_POP,      ///< pop call message
} test_com_data_id;

/// test com data
typedef union test_com_data
{
    test_com_pos_st         pos;        ///< position message
    test_com_msg_add_st     msg_add;    ///< add text message
    test_com_msg_append_st  msg_append; ///< append text message
    test_com_call_push_st   call_push;  ///< push call message
} test_com_data_un;

/// test com data callback
///
/// \param type     data type
/// \param data     data
/// \param ctx      callback context
typedef void (*test_com_data_cb)(test_com_data_id type, const test_com_data_un *data, void *ctx);


/// Send position message.
///
/// \param com      com socket
/// \param file     file name
/// \param line     file line
/// \param after    position is after line
///
/// \retval 0               success
/// \retval -1/GEN/errno    send error
int test_com_set_pos(int com, const char *file, size_t line, bool after);

/// Send text message.
///
/// \param com      com socket
/// \param type     message type
/// \param msg      message
///
/// \retval 0               success
/// \retval -1/GEN/errno    send error
int test_com_add_msg(int com, test_msg_id type, const char *msg);

/// Send text message.
///
/// \param com      com socket
/// \param level    message level
/// \param msg      message, may be NULL
///
/// \retval 0               success
/// \retval -1/GEN/errno    send error
int test_com_append_msg(int com, int level, const char *msg);

/// Send push call message.
///
/// \param com      com socket
/// \param call     call
///
/// \retval 0               success
/// \retval -1/GEN/errno    send error
int test_com_push_call(int com, const char *call);

/// Send pop call message.
///
/// \param com      com socket
///
/// \retval 0               success
/// \retval -1/GEN/errno    send error
int test_com_pop_call(int com);

/// Receive one message from com socket and invoke callback on it.
///
/// \param com      com socket
/// \param cb       data callback
/// \param ctx      callback context
///
/// \retval 0                   success
/// \retval -1/GEN/ENOMEM       out of memory
/// \retval -1/GEN/ENODATA      EOF
/// \retval -1/GEN/EWOULDBLOCK  recv would block
/// \retval -1/GEN/errno        recv error
int test_com_recv_msg(int com, test_com_data_cb cb, const void *ctx);

/// Receive all messages from com socket and invoke callback on them.
///
/// \param com      com socket
/// \param cb       data callback
/// \param ctx      callback context
///
/// \retval 0               success
/// \retval -1/GEN/ENOMEM   out of memory
/// \retval -1/GEN/ENODATA  EOF
/// \retval -1/GEN/errno    recv error
int test_com_recv(int com, test_com_data_cb cb, const void *ctx);

/// Clear test com receive buffer.
///
///
void test_com_clear(void);

/// Free test com receive buffer.
///
///
void test_com_free(void);


#endif // ifndef YTIL_TEST_COM_H_INCLUDED
