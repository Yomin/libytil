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

#include "com.h"
#include <ytil/def.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#if OS_WINDOWS
    #include <winsock2.h>
#else
    #include <sys/socket.h>
#endif


/// test com buffer
typedef struct test_com_buf
{
    char    *data;  ///< buffer data
    size_t  cap;    ///< buffer capacity
    size_t  size;   ///< number of bytes in buffer
    size_t  pos;    ///< recv position
} test_com_buf_st;

/// global test com buffer
static test_com_buf_st *buf;


/// Write data to com socket.
///
/// \param com      com socket
/// \param vdata    data to send
/// \param size     size of data
///
/// \retval 0               success
/// \retval -1/GEN/errno    errno
static int test_com_write(int com, void *vdata, size_t size)
{
    const char *data = vdata;
    ssize_t count;

    for(; size; data += count, size -= count)
    {
        if((count = send(com, data, size, 0)) < 0)
            return error_pass_last_errno(send), -1;
    }

    return 0;
}

/// Send message over com socket.
///
/// \param com      com socket
/// \param type     data type
/// \param ...      variadic list of (data, size) tuples
///
/// \retval 0               success
/// \retval -1/GEN/errno    errno
static int test_com_send(int com, test_com_data_id type, ...)
{
    va_list ap;
    void *data;
    size_t size;

    assert(com >= 0);

    if(test_com_write(com, &type, sizeof(test_com_data_id)))
        return error_pass(), -1;

    va_start(ap, type);

    while((data = va_arg(ap, void *)))
    {
        size = va_arg(ap, size_t);

        if(test_com_write(com, data, size))
            return error_pass(), va_end(ap), -1;
    }

    va_end(ap);

    return 0;
}

int test_com_set_pos(int com, const char *file, size_t line, bool after)
{
    size_t len;

    assert(file);

    len = strlen(file);

    return error_pass_int(test_com_send(com, TEST_COM_POS,
        &line, sizeof(size_t),
        &after, sizeof(bool),
        &len, sizeof(size_t),
        file, len,
        NULL));
}

int test_com_add_msg(int com, test_msg_id type, const char *msg)
{
    size_t len;

    assert(type < TEST_MSG_TYPES);
    assert(msg);

    len = strlen(msg);

    return error_pass_int(test_com_send(com, TEST_COM_MSG_ADD,
        &type, sizeof(test_msg_id),
        &len, sizeof(size_t),
        msg, len,
        NULL));
}

int test_com_append_msg(int com, int level, const char *msg)
{
    size_t len;

    assert(level >= 0);

    msg = msg ? msg : "";
    len = strlen(msg);

    return error_pass_int(test_com_send(com, TEST_COM_MSG_APPEND,
        &level, sizeof(int),
        &len, sizeof(size_t),
        msg, len,
        NULL));
}

int test_com_push_call(int com, const char *call)
{
    size_t len;

    assert(call);

    len = strlen(call);

    return error_pass_int(test_com_send(com, TEST_COM_CALL_PUSH,
        &len, sizeof(size_t),
        call, len,
        NULL));
}

int test_com_pop_call(int com)
{
    return error_pass_int(test_com_send(com, TEST_COM_CALL_POP, NULL));
}

/// Read data from com socket.
///
/// \param com     com socket
/// \param data    buffer to fill, may be NULL, buf->pos is not advanced
/// \param size    number of bytes to receive
///
/// \retval 0                   success
/// \retval -1/GEN/ENOMEM       out of memory
/// \retval -1/GEN/ENODATA      EOF
/// \retval -1/GEN/EWOULDBLOCK  recv would block
/// \retval -1/GEN/errno        recv error
static int test_com_read(int com, void *data, size_t size)
{
    char *tmp, *rdata;
    size_t cap, rsize;
    ssize_t count;

    if(!buf->data || buf->pos + size > buf->cap)
    {
        cap = MAX3(64U, buf->size + size, buf->cap * 2);

        if(!(tmp = realloc(buf->data, cap + 1)))
            return error_pass_last_errno(realloc), -1;

        buf->data   = tmp;
        buf->cap    = cap;
    }

    if(buf->pos + size > buf->size)
    {
        rsize   = size - (buf->size - buf->pos);
        rdata   = &buf->data[buf->size];

        for(; rsize; rdata += count, rsize -= count, buf->size += count)
        {
            if((count = recv(com, rdata, rsize, 0)) < 0)
                return error_pass_last_errno(recv), -1;
            else if(!count)
                return error_set_s(ERRNO, ENODATA), -1; // EOF
        }

        rdata[0] = '\0'; // always terminate buffer
    }

    if(data)
    {
        memcpy(data, &buf->data[buf->pos], size);
        buf->pos += size;
    }

    return 0;
}

/// Read text from com socket (len, text).
///
/// \warning Returns a pointer into the com buffer.
///          Use only to read last message property.
///
/// \param com      com socket
///
/// \returns                        text
/// \retval NULL/GEN/ENOMEM         out of memory
/// \retval NULL/GEN/ENODATA        EOF
/// \retval NULL/GEN/EWOULDBLOCK    recv would block
/// \retval NULL/GEN/errno          recv error
static char *test_com_read_text(int com)
{
    size_t len;
    char *text;

    if(test_com_read(com, &len, sizeof(size_t)))
        return error_pass(), NULL;

    if(test_com_read(com, NULL, len))
        return error_pass(), NULL;

    text        = &buf->data[buf->pos];
    buf->pos    += len;

    return text;
}

int test_com_recv_msg(int com, test_com_data_cb cb, const void *ctx)
{
    test_com_data_id type;
    test_com_data_un data = { 0 };

    assert(com >= 0);
    assert(cb);

    if(!buf && !(buf = calloc(1, sizeof(test_com_buf_st))))
        return error_pass_last_errno(calloc), -1;

    buf->pos = 0;

    if(test_com_read(com, &type, sizeof(test_com_data_id)))
        return error_pass(), -1;

    switch(type)
    {
    case TEST_COM_POS:

        if(test_com_read(com, &data.pos.line, sizeof(size_t)))
            return error_pass(), -1;

        if(test_com_read(com, &data.pos.after, sizeof(bool)))
            return error_pass(), -1;

        if(!(data.pos.file = test_com_read_text(com)))
            return error_pass(), -1;

        break;

    case TEST_COM_MSG_ADD:

        if(test_com_read(com, &data.msg_add.type, sizeof(test_msg_id)))
            return error_pass(), -1;

        if(!(data.msg_add.msg = test_com_read_text(com)))
            return error_pass(), -1;

        break;

    case TEST_COM_MSG_APPEND:

        if(test_com_read(com, &data.msg_append.level, sizeof(int)))
            return error_pass(), -1;

        if(!(data.msg_append.msg = test_com_read_text(com)))
            return error_pass(), -1;

        break;

    case TEST_COM_CALL_PUSH:

        if(!(data.call_push.call = test_com_read_text(com)))
            return error_pass(), -1;

        break;

    case TEST_COM_CALL_POP:
        break;

    default:
        abort();
    }

    cb(type, &data, (void *)ctx);

    if(buf->pos != buf->size)
        abort();

    buf->size = 0;

    return 0;
}

int test_com_recv(int com, test_com_data_cb cb, const void *ctx)
{
    while(1)
    {
        if(test_com_recv_msg(com, cb, ctx))
            break;
    }

    if(error_code(0) == EWOULDBLOCK)
        return 0;

    return error_pass(), -1;
}

void test_com_clear(void)
{
    if(buf)
        buf->size = 0;
}

void test_com_free(void)
{
    if(!buf)
        return;

    free(buf->data);
    free(buf);
    buf = NULL;
}
